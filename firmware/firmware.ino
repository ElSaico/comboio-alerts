#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>

#include "fonts.hpp"

#define DEBUG         0

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

#define ZONE_LABEL(z) labels[(z)-1]
#define ZONE_END(z)   (SIZE_ALERTS + SIZE_LABEL*(z) - 1)
#define ZONE_BEGIN(z) (ZONE_END(z) - ((z) ? SIZE_LABEL : SIZE_ALERTS) + 1)

#define LABEL_SPEED   50

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
char user[MAX_LABEL];
char message[MAX_MESSAGE];
#if DEBUG
char debugBuffer[MAX_LABEL];
#endif

void setup() {
  Serial.begin(BAUD_RATE);
  P.begin(NUM_ZONES);
  for (uint8_t z = 0; z < NUM_ZONES; z++) {
    P.setZone(z, ZONE_BEGIN(z), ZONE_END(z));
  }
  for (uint8_t z = 1; z < NUM_ZONES; z++) {
    P.setFont(z, fontMetro);
    P.setTextBuffer(z, ZONE_LABEL(z));
    P.setPause(z, LABEL_SPEED);
    P.setSpeed(z, LABEL_SPEED);
    P.setTextAlignment(z, PA_CENTER);
  }
}

void onAlertBar(EventType eventType, uint32_t num, char* user, char* message = nullptr) {
  // TODO set state and initial message
  P.displayReset(ZONE_ALERTS);
}

void setLabelAnimation(MatrixZone z) {
  if (P.getTextColumns(z, ZONE_LABEL(z)) > SIZE_LABEL*8) {
    P.setTextEffect(z, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
  } else {
    P.setTextEffect(z, PA_PRINT, PA_NO_EFFECT);
  }
  P.displayReset(z);
}

void onLabel(EventType eventType, uint32_t num, char* user) {
  switch (eventType) {
    case EVENT_FOLLOW:
      strncpy(ZONE_LABEL(ZONE_FOLLOW), user, MAX_LABEL);
      setLabelAnimation(ZONE_FOLLOW);
      break;
    case EVENT_SUB_NEW: case EVENT_SUB_RENEW:
      snprintf_P(ZONE_LABEL(ZONE_SUB), MAX_LABEL, PSTR("%s (%d)"), user, num);
      setLabelAnimation(ZONE_SUB);
      break;
    case EVENT_CHEER:
      snprintf_P(ZONE_LABEL(ZONE_CHEER), MAX_LABEL, PSTR("%s (%d)"), user, num);
      setLabelAnimation(ZONE_CHEER);
      break;
    default: ;
#if DEBUG
      sprintf_P(debugBuffer, PSTR("invalid e=%c for t=L\n"), eventType);
      Serial.write(debugBuffer);
#endif
  }
}

void readSerial() {
  static InputStage stage = STAGE_TARGET;
  static char target;
  static EventType eventType;
  static uint32_t num; // talk about future-proofing; are we ever getting a raid > 65535?
  static uint8_t userSize;
  static uint16_t messageSize;

  char rc = Serial.read();
  switch (stage) {
    case STAGE_TARGET:
      if (rc == TARGET_ALERTS || rc == TARGET_LABEL) {
        target = rc;
        stage = STAGE_EVENT;
#if DEBUG
        sprintf_P(debugBuffer, PSTR("t=%c"), rc);
      } else {
        sprintf_P(debugBuffer, PSTR("invalid t=%c\n"), rc);
#endif
      }
#if DEBUG
      Serial.write(debugBuffer);
#endif
      break;
    case STAGE_EVENT:
      eventType = (EventType)rc;
      stage = STAGE_NUMBER;
      num = 0;
#if DEBUG
      sprintf_P(debugBuffer, PSTR(" e=%c"), rc);
      Serial.write(debugBuffer);
#endif
      break;
    case STAGE_NUMBER:
      if (rc != '\n') {
        num *= 10;
        num += rc - '0';
      } else {
        stage = STAGE_USER;
        userSize = 0;
#if DEBUG
        sprintf_P(debugBuffer, PSTR(" n=%d\n"), num);
        Serial.write(debugBuffer);
#endif
      }
      break;
    case STAGE_USER:
      if (rc != '\n') {
        user[userSize++] = rc;
      } else {
        user[userSize] = '\0';
#if DEBUG
        sprintf_P(debugBuffer, PSTR("u=%s\n"), user);
        Serial.write(debugBuffer);
#endif
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
#if DEBUG
              sprintf_P(debugBuffer, PSTR("invalid e=%s\n"), eventType);
              Serial.write(debugBuffer);
#endif
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
    default: ;
#if DEBUG
      sprintf_P(debugBuffer, PSTR("invalid stage %d (WTF?)\n"), stage);
      Serial.write(debugBuffer);
#endif
  }
}

void loop() {
  if (Serial.available()) {
    readSerial();
  }

  if (P.displayAnimate()) {
    if (P.getZoneStatus(ZONE_ALERTS)) {
      // TODO implement a FSM like readSerial (alert, user, message)
      // send state updates to the bot (how?), for TTS and queue unlock
    }
    for (uint8_t i = 1; i < NUM_ZONES; i++) {
      if (P.getZoneStatus(i)) {
        P.displayReset(i);
      }
    }
  }
}
