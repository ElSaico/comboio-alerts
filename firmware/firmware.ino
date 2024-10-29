#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>

#include "fonts.hpp"

#define BAUD_RATE     9600
#define CS_PIN        10

#define NUM_ZONES     4
#define NUM_MODULES   54

#define MAX_LABEL     36 // up to 25 for username + 4 for " ()\0"; this leaves a minimum of 7 for amount
#define MAX_MESSAGE   512

#define SIZE_ALERTS   24
#define SIZE_LABEL    10

#define TARGET_ALERTS 'A'
#define TARGET_LABEL  'L'

const uint8_t ZONES[] PROGMEM = {
  0,
  SIZE_ALERTS,
  SIZE_ALERTS + SIZE_LABEL,
  SIZE_ALERTS + SIZE_LABEL*2,
  SIZE_ALERTS + SIZE_LABEL*3
};

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
char output[MAX_LABEL];

void setup() {
  Serial.begin(BAUD_RATE);
  P.begin(NUM_ZONES);
  for (uint8_t i = 0; i < NUM_ZONES; i++) {
    P.setZone(i, pgm_read_byte_near(ZONES+i), pgm_read_byte_near(ZONES+i+1)-1);
  }
  for (uint8_t i = 1; i < NUM_ZONES; i++) {
    P.setFont(i, fontMetro);
    P.displayZoneText(i, labels[i-1], PA_LEFT, 100, 1000, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
  }
}

void onAlertBar(EventType eventType, uint32_t num, char* user, char* message = nullptr) {
}

void onLabel(EventType eventType, uint32_t num, char* user) {
  switch (eventType) {
    case EVENT_FOLLOW:
      strncpy(labels[ZONE_FOLLOW-1], user, MAX_LABEL);
      P.displayReset(ZONE_FOLLOW);
      break;
    case EVENT_SUB_NEW: case EVENT_SUB_RENEW:
      snprintf_P(labels[ZONE_SUB-1], MAX_LABEL, PSTR("%s (%d)"), user, num);
      P.displayReset(ZONE_SUB);
      break;
    case EVENT_CHEER:
      snprintf_P(labels[ZONE_CHEER-1], MAX_LABEL, PSTR("%s (%d)"), user, num);
      P.displayReset(ZONE_CHEER);
      break;
    default:
      sprintf_P(output, PSTR("invalid e=%c for t=L\n"), eventType);
      Serial.write(output);
  }
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
        sprintf_P(output, PSTR("t=%c"), rc);
      } else {
        sprintf_P(output, PSTR("invalid t=%c\n"), rc);
      }
      Serial.write(output);
      break;
    case STAGE_EVENT:
      eventType = (EventType)rc;
      stage = STAGE_NUMBER;
      num = 0;
      sprintf_P(output, PSTR(" e=%c"), rc);
      Serial.write(output);
      break;
    case STAGE_NUMBER:
      if (rc != '\n') {
        num *= 10;
        num += rc - '0';
      } else {
        stage = STAGE_USER;
        userSize = 0;
        sprintf_P(output, PSTR(" n=%d\n"), num);
        Serial.write(output);
      }
      break;
    case STAGE_USER:
      if (rc != '\n') {
        user[userSize++] = rc;
      } else {
        user[userSize] = '\0';
        sprintf_P(output, PSTR("u=%s\n"), user);
        Serial.write(output);
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
              sprintf_P(output, PSTR("invalid e=%s\n"), eventType);
              Serial.write(output);
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
      }
      break;
    default:
      sprintf_P(output, PSTR("invalid stage %d (WTF?)\n"), stage);
      Serial.write(output);
  }
}

void loop() {
  if (Serial.available()) {
    readSerial();
  }

  if (P.displayAnimate()) {
    for (uint8_t i = 0; i < NUM_ZONES; i++) {
      if (P.getZoneStatus(i)) {
        P.displayReset(i);
      }
    }
  }
}
