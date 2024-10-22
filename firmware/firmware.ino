#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <Streaming.h>
#include <SPI.h>

#include "fonts.hpp"

#define BAUD_RATE     9600
#define HARDWARE_TYPE MD_MAX72XX::PAROLA_HW
#define CS_PIN        10
#define NUM_MODULES   54

#define MAX_LABEL     36 // up to 25 for username + 4 for " ()\0"; this leaves a minimum of 7 for amount
#define MAX_MESSAGE   512

#define NUM_ZONES     4
#define ZONE_FOLLOW   1
#define ZONE_SUB      2
#define ZONE_CHEER    3

#define TARGET_ALERTS 'A'
#define TARGET_LABEL  'L'

const uint8_t ZONES[] PROGMEM = { 0, 24, 34, 44, 54 };

enum Stage { TARGET, EVENT_TYPE, NUMBER, USERNAME, MESSAGE };
enum EventType : char {
  FOLLOWER = 'F',
  SUB_NEW = 'S',
  SUB_RENEW = 's',
  SUB_GIFT = 'G',
  CHEER = 'C',
  RAID = 'R',
};

auto P = MD_Parola(HARDWARE_TYPE, CS_PIN, NUM_MODULES);
uint8_t flushZones = 0;
char labels[NUM_ZONES-1][MAX_LABEL];

void setup() {
  Serial.begin(BAUD_RATE);
  P.begin(NUM_ZONES);
  for (uint8_t i = 0; i < NUM_ZONES; i++) {
    P.setZone(i, ZONES[i], ZONES[i+1]-1);
  }
  for (uint8_t i = 1; i < NUM_ZONES; i++) {
    P.displayZoneText(i, labels[i-1], PA_LEFT, 60, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
  }
}

void onAlertBar(EventType eventType, uint32_t num, char* user, char* message = nullptr) {
}

void onLabel(EventType eventType, uint32_t num, char* user) {
  uint8_t zone;
  switch (eventType) {
    case FOLLOWER:
      zone = ZONE_FOLLOW;
      break;
    case SUB_NEW: case SUB_RENEW:
      zone = ZONE_SUB;
      break;
    case CHEER:
      zone = ZONE_CHEER;
      break;
    default:
      Serial << "invalid e=" << eventType << " for t=L" << endl;
      return;
  }
  strncpy_P(labels[zone-1], user, MAX_LABEL);
  flushZones |= 1 << zone;
}

void readSerial() {
  static Stage stage = TARGET;
  static char target;
  static EventType eventType;
  static uint32_t num; // talk about future-proofing; are we ever getting a raid > 65535?
  static char user[MAX_LABEL];
  static uint8_t userSize;
  static char message[MAX_MESSAGE];
  static uint16_t messageSize;

  char rc = Serial.read();
  switch (stage) {
    case TARGET:
      if (rc == TARGET_ALERTS || rc == TARGET_LABEL) {
        target = rc;
        stage = EVENT_TYPE;
        Serial << "t=" << target;
      } else {
        Serial << "invalid t=" << rc << endl;
      }
      break;
    case EVENT_TYPE:
      eventType = (EventType)rc;
      stage = NUMBER;
      num = 0;
      Serial << " e=" << rc;
      break;
    case NUMBER:
      if (rc != '\n') {
        num *= 10;
        num += rc - '0';
      } else {
        stage = USERNAME;
        userSize = 0;
        Serial << " n=" << num << endl;
      }
      break;
    case USERNAME:
      if (rc != '\n') {
        user[userSize++] = rc;
      } else {
        user[userSize] = '\0';
        Serial << "u=" << user << endl;
        if (target == TARGET_ALERTS) {
          switch (eventType) {
            case FOLLOWER: case SUB_GIFT:
              stage = TARGET;
              onAlertBar(eventType, num, user);
              break;
            case SUB_NEW: case SUB_RENEW: case CHEER: case RAID:
              stage = MESSAGE;
              messageSize = 0;
              break;
            default:
              stage = TARGET;
              Serial << "invalid e=" << eventType << endl;
          }
        } else {
          stage = TARGET;
          onLabel(eventType, num, user);
        }
      }
      break;
    case MESSAGE:
      if (rc != '\n') {
        message[messageSize++] = rc;
      } else {
        message[messageSize] = '\0';
        stage = TARGET;
        onAlertBar(eventType, num, user, message);
        Serial << "m=" << message << endl;
      }
      break;
    default:
      Serial << "invalid stage " << stage << " (WTF?)" << endl;
  }
}

void loop() {
  if (Serial.available()) {
    readSerial();
  }

  if (P.displayAnimate()) {
    for (uint8_t i = 1; i < NUM_ZONES; i++) {
      if (P.getZoneStatus(i)) {
        P.displayReset(i);
      }
      if (flushZones & (1 << i)) {
        flushZones &= ~(1 << i);
        Serial << "reset label " << i << endl;
      }
    }
  }
}
