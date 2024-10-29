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

auto P = MD_Parola(MD_MAX72XX::PAROLA_HW, CS_PIN, NUM_MODULES);
char userBuffer[MAX_LABEL];
char messageBuffer[MAX_MESSAGE];

auto alerts = AlertDisplay(&P, userBuffer, messageBuffer);
auto labelFollow = LabelDisplay(&P, ZONE_FOLLOW);
auto labelSub = LabelDisplay(&P, ZONE_SUB);
auto labelCheer = LabelDisplay(&P, ZONE_CHEER);

void setup() {
  Serial.begin(BAUD_RATE);
  P.begin(NUM_ZONES);
  alerts.begin();
  labelFollow.begin();
  labelSub.begin();
  labelCheer.begin();
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
        userBuffer[userSize++] = rc;
      } else {
        userBuffer[userSize] = '\0';
        if (target == TARGET_ALERTS) {
          switch (eventType) {
            case EVENT_FOLLOW: case EVENT_SUB_GIFT:
              stage = INPUT_TARGET;
              alerts.set(eventType, num);
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
          label->set(userBuffer, num);
        }
      }
      break;
    case INPUT_MESSAGE:
      if (rc != '\n') {
        messageBuffer[messageSize++] = rc;
      } else {
        messageBuffer[messageSize] = '\0';
        stage = INPUT_TARGET;
        alerts.set(eventType, num);
      }
      break;
  }
}

void loop() {
  if (Serial.available()) {
    readSerial();
  }

  if (P.displayAnimate()) {
    alerts.animate();
    labelFollow.animate();
    labelSub.animate();
    labelCheer.animate();
  }
}
