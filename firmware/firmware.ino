#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <Streaming.h>
#include <SPI.h>

#include "fonts.hpp"

#define BAUD_RATE     9600
#define CS_PIN        10

#define NUM_ZONES     4
#define NUM_MODULES   54

#define MAX_LABEL     36 // up to 25 for username + 4 for " ()\0"; this leaves a minimum of 7 for amount
#define MAX_MESSAGE   512

#define TARGET_ALERTS 'A'
#define TARGET_LABEL  'L'

const uint8_t ZONES[] = { 0, 24, 34, 44, 54 };

enum InputStage { STAGE_TARGET, STAGE_EVENT, STAGE_NUMBER, STAGE_USER, STAGE_MESSAGE };
enum MatrixZone { ZONE_ALERTS, ZONE_FOLLOW, ZONE_SUB, ZONE_CHEER };
enum EventType : char {
  EVENT_FOLLOW = 'F',
  EVENT_SUB_NEW = 'S',
  EVENT_SUB_RENEW = 's',
  EVENT_SUB_GIFT = 'G',
  EVENT_CHEER = 'C',
  EVENT_RAID = 'R',
};

auto P = MD_Parola(MD_MAX72XX::PAROLA_HW, CS_PIN, NUM_MODULES);
char labels[NUM_ZONES-1][MAX_LABEL];

void setup() {
  Serial.begin(BAUD_RATE);
  P.begin(NUM_ZONES);
  for (uint8_t i = 0; i < NUM_ZONES; i++) {
    P.setZone(i, ZONES[i], ZONES[i+1]-1);
  }
}

void onAlertBar(EventType eventType, uint32_t num, char* user, char* message = nullptr) {
}

void onLabel(EventType eventType, uint32_t num, char* user) {
  MatrixZone zone;
  switch (eventType) {
    case EVENT_FOLLOW:
      zone = ZONE_FOLLOW;
      break;
    case EVENT_SUB_NEW: case EVENT_SUB_RENEW:
      zone = ZONE_SUB;
      break;
    case EVENT_CHEER:
      zone = ZONE_CHEER;
      break;
    default:
      Serial << "invalid e=" << eventType << " for t=L" << endl;
      return;
  }
  strncpy(labels[zone-1], user, MAX_LABEL);
  P.displayZoneText(zone, labels[zone-1], PA_LEFT, 100, 1000, PA_SCROLL_LEFT);
  P.displayReset(zone);
}

void readSerial() {
  static InputStage stage = STAGE_TARGET;
  static char target;
  static EventType eventType;
  static uint32_t num; // talk about future-proofing; are we ever getting a raid > 65535?
  static char user[MAX_LABEL];
  static uint8_t userSize;
  static char message[MAX_MESSAGE];
  static uint16_t messageSize;

  char rc = Serial.read();
  switch (stage) {
    case STAGE_TARGET:
      if (rc == TARGET_ALERTS || rc == TARGET_LABEL) {
        target = rc;
        stage = STAGE_EVENT;
        Serial << "t=" << target;
      } else {
        Serial << "invalid t=" << rc << endl;
      }
      break;
    case STAGE_EVENT:
      eventType = (EventType)rc;
      stage = STAGE_NUMBER;
      num = 0;
      Serial << " e=" << rc;
      break;
    case STAGE_NUMBER:
      if (rc != '\n') {
        num *= 10;
        num += rc - '0';
      } else {
        stage = STAGE_USER;
        userSize = 0;
        Serial << " n=" << num << endl;
      }
      break;
    case STAGE_USER:
      if (rc != '\n') {
        user[userSize++] = rc;
      } else {
        user[userSize] = '\0';
        Serial << "u=" << user << endl;
        if (target == TARGET_ALERTS) {
          switch (eventType) {
            case EVENT_FOLLOW: case EVENT_SUB_GIFT:
              stage = STAGE_TARGET;
              onAlertBar(eventType, num, user);
              break;
            case EVENT_SUB_NEW: case EVENT_SUB_RENEW: case EVENT_CHEER: case EVENT_RAID:
              stage = STAGE_MESSAGE;
              messageSize = 0;
              break;
            default:
              stage = STAGE_TARGET;
              Serial << "invalid e=" << eventType << endl;
          }
        } else {
          stage = STAGE_TARGET;
          onLabel(eventType, num, user);
        }
      }
      break;
    case STAGE_MESSAGE:
      if (rc != '\n') {
        message[messageSize++] = rc;
      } else {
        message[messageSize] = '\0';
        stage = STAGE_TARGET;
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

  P.displayAnimate();
  for (uint8_t i = 0; i < NUM_ZONES; i++) {
    if (P.getZoneStatus(i)) {
      P.displayReset(i);
    }
  }
}
