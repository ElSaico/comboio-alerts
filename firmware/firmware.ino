#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <Streaming.h>
#include <SPI.h>

#include "fonts.hpp"

#define HARDWARE_TYPE MD_MAX72XX::PAROLA_HW
#define MAX_DEVICES   54
#define CS_PIN        10
#define NUM_ZONES     4

enum Stage { TARGET, EVENT_TYPE, NUMBER, USERNAME, MESSAGE };
enum EventType : char {
  FOLLOWER = 'F',
  SUB_NEW = 'S',
  SUB_RENEW = 's',
  SUB_GIFT = 'G',
  CHEER = 'C',
  RAID = 'R',
};

auto P = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);
uint8_t ZONES[] = { 0, 24, 34, 44, 54 };
uint8_t flushZones = 0;
String labels[NUM_ZONES-1];

void setup() {
  Serial.begin(9600);
  P.begin(NUM_ZONES);
  for (uint8_t i = 0; i < NUM_ZONES; i++) {
    P.setZone(i, ZONES[i], ZONES[i+1]-1);
  }
}

void onAlertBar(EventType eventType, uint32_t num, char* user, char* message = nullptr) {
}

void onLabel(EventType eventType, uint32_t num, char* user) {
  uint8_t zone;
  switch (eventType) {
    case FOLLOWER:
      zone = 1;
      break;
    case SUB_NEW: case SUB_RENEW:
      zone = 2;
      break;
    case CHEER:
      zone = 3;
      break;
    default:
      Serial << "invalid e=" << eventType << " for t=L" << endl;
      return;
  }
  P.displayZoneText(zone, user, PA_LEFT, 500, 0, PA_SCROLL_LEFT);
  flushZones |= 1 << zone;
}

void readSerial() {
  static Stage stage = TARGET;
  static char target;
  static EventType eventType;
  static uint32_t num; // talk about future-proofing; are we ever getting a raid > 65535?
  static char user[64]; // TODO check Twitch limit
  static uint8_t userSize;
  static char message[512]; // TODO check Twitch limit
  static uint16_t messageSize;

  char rc = Serial.read();
  switch (stage) {
    case TARGET:
      if (rc == 'A' || rc == 'L') {
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
        if (target == 'A') {
          switch (eventType) {
            case FOLLOWER: case SUB_GIFT: case RAID:
              stage = TARGET;
              onAlertBar(eventType, num, user);
              break;
            case SUB_NEW: case SUB_RENEW: case CHEER:
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
      if (flushZones & (1 << i)) {
        P.displayReset(i);
      }
    }
  }
}
