#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>

#include "display.hpp"
#include "fonts.hpp"

#define BAUD_RATE     9600
#define CS_PIN        10

#define NUM_ZONES     4
#define NUM_MODULES   54

#define TARGET_ALERTS 'A'
#define TARGET_LABEL  'L'

enum InputStage { INPUT_TARGET, INPUT_EVENT, INPUT_NUMBER, INPUT_USER, INPUT_MESSAGE };
enum AlertStage { ALERT_MAIN, ALERT_USER, ALERT_MESSAGE };
enum EventType : char {
  EVENT_FOLLOW = 'F',
  EVENT_SUB_NEW = 'S',
  EVENT_SUB_RENEW = 's',
  EVENT_SUB_GIFT = 'G',
  EVENT_CHEER = 'C',
  EVENT_RAID = 'R',
  EVENT_DONATE = 'D',
  EVENT_SHOUTOUT = 'O',
};

auto P = MD_Parola(MD_MAX72XX::PAROLA_HW, CS_PIN, NUM_MODULES);

auto labelFollow = LabelDisplay(&P, LabelDisplay::ZONE_FOLLOW);
auto labelSub = LabelDisplay(&P, LabelDisplay::ZONE_SUB);
auto labelCheer = LabelDisplay(&P, LabelDisplay::ZONE_CHEER);

AlertStage alertStage;
char alert[MAX_MESSAGE];
char user[MAX_LABEL];
char message[MAX_MESSAGE];

void setup() {
  Serial.begin(BAUD_RATE);
  P.begin(NUM_ZONES);
  P.setZone(LabelDisplay::ZONE_ALERTS, ZONE_BEGIN(LabelDisplay::ZONE_ALERTS), ZONE_END(LabelDisplay::ZONE_ALERTS));
  P.setSpeed(LabelDisplay::ZONE_ALERTS, LABEL_SPEED);
  P.setTextAlignment(LabelDisplay::ZONE_ALERTS, PA_CENTER);
  P.setPause(LabelDisplay::ZONE_ALERTS, 0);
  labelFollow.begin();
  labelSub.begin();
  labelCheer.begin();
}

void onAlertBar(EventType eventType, uint32_t num, bool withMessage) {
  alertStage = ALERT_MAIN;
  P.setFont(LabelDisplay::ZONE_ALERTS, FONT_METRO);
  P.setTextBuffer(LabelDisplay::ZONE_ALERTS, alert);
  P.setTextEffect(LabelDisplay::ZONE_ALERTS, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
  P.displayReset(LabelDisplay::ZONE_ALERTS);
}

void readSerial() {
  static InputStage stage = INPUT_TARGET;
  static char target;
  static EventType eventType;
  static uint32_t num; // way too optimistic to support a raid of over 65535, but it's only 2 extra bytes
  static uint8_t userSize;
  static uint16_t messageSize;

  char rc = Serial.read();
  switch (stage) {
    case INPUT_TARGET:
      if (rc == TARGET_ALERTS || rc == TARGET_LABEL) {
        target = rc;
        stage = INPUT_EVENT;
      }
      break;
    case INPUT_EVENT:
      eventType = (EventType)rc;
      stage = INPUT_NUMBER;
      num = 0;
      break;
    case INPUT_NUMBER:
      if (rc != '\n') {
        num *= 10;
        num += rc - '0';
      } else {
        stage = INPUT_USER;
        userSize = 0;
      }
      break;
    case INPUT_USER:
      if (rc != '\n') {
        user[userSize++] = rc;
      } else {
        user[userSize] = '\0';
        if (target == TARGET_ALERTS) {
          switch (eventType) {
            case EVENT_FOLLOW: case EVENT_SUB_GIFT:
              stage = INPUT_TARGET;
              onAlertBar(eventType, num, false);
              break;
            case EVENT_SUB_NEW: case EVENT_SUB_RENEW: case EVENT_CHEER: case EVENT_RAID:
              stage = INPUT_MESSAGE;
              messageSize = 0;
              break;
            default:
              stage = INPUT_TARGET;
          }
        } else {
          stage = INPUT_TARGET;
          LabelDisplay* label;
          switch (eventType) {
            case EVENT_FOLLOW:
              label = &labelFollow;
              break;
            case EVENT_SUB_NEW: case EVENT_SUB_RENEW: case EVENT_SUB_GIFT:
              label = &labelSub;
              break;
            case EVENT_CHEER:
              label = &labelCheer;
              break;
            default:
              return;
          }
          label->set(user, num);
        }
      }
      break;
    case INPUT_MESSAGE:
      if (rc != '\n') {
        message[messageSize++] = rc;
      } else {
        message[messageSize] = '\0';
        stage = INPUT_TARGET;
        onAlertBar(eventType, num, true);
      }
      break;
  }
}

void loop() {
  if (Serial.available()) {
    readSerial();
  }

  if (P.displayAnimate()) {
    if (P.getZoneStatus(LabelDisplay::ZONE_ALERTS)) {
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
