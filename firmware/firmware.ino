#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>

#include "fonts.hpp"

#define BAUD_RATE      9600
#define CS_PIN         10

#define NUM_ZONES      4
#define NUM_MODULES    54

#define MAX_LABEL      36 // up to 25 for username + 4 for " ()\0"; this leaves a minimum of 7 for amount
#define MAX_ALERT      64
#define MAX_MESSAGE    512

#define SIZE_ALERTS    24
#define SIZE_LABEL     10

#define LABEL_SPEED    50

#define LABEL_START(z) (SIZE_ALERTS + SIZE_LABEL*((z) - 1))
#define LABEL_END(z)   (SIZE_ALERTS + SIZE_LABEL*(z) - 1)

#define TARGET_ALERTS 'A'
#define TARGET_LABEL  'L'

enum InputStage { INPUT_TARGET, INPUT_EVENT, INPUT_NUMBER, INPUT_USER, INPUT_MESSAGE };
enum MatrixZone { ZONE_ALERTS, ZONE_FOLLOW, ZONE_SUB, ZONE_CHEER };
enum AlertStage { ALERT_IDLE, ALERT_EVENT, ALERT_USER, ALERT_MESSAGE };
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

const char ALERTS_FOLLOW[] PROGMEM = "Novo passageiro no Comboio";
// TODO pluralize
const char ALERTS_RAID[] PROGMEM = "Embarque de uma raid com %d pessoas";
const char ALERTS_SUB_GIFT[] PROGMEM = "%d passes doados para o Comboio";
const char ALERTS_SUB_NEW[] PROGMEM = "Nova aquisição de passe";
const char ALERTS_SUB_RENEW[] PROGMEM = "Renovação de passe, totalizando %d meses";
// TODO check how to handle decimal values
const char ALERTS_DONATE[] PROGMEM = "Pix de %s enviado para o Comboio";
const char ALERTS_SHOUTOUT[] PROGMEM = "O Comboio do Saico recomenda este canal";

auto P = MD_Parola(MD_MAX72XX::PAROLA_HW, CS_PIN, NUM_MODULES);
char userBuffer[MAX_LABEL];
char messageBuffer[MAX_MESSAGE];
char labelBuffer[NUM_ZONES-1][MAX_LABEL];
char alertBuffer[MAX_ALERT];
bool alertMessage;
auto alertStage = ALERT_IDLE;

void beginAlert() {
  P.setZone(ZONE_ALERTS, 0, SIZE_ALERTS-1);
  P.setSpeed(ZONE_ALERTS, LABEL_SPEED);
  P.setTextAlignment(ZONE_ALERTS, PA_CENTER);
  P.setPause(ZONE_ALERTS, 0);
}

void beginLabel(MatrixZone z) {
  P.setZone(z, LABEL_START(z), LABEL_END(z));
  P.setSpeed(z, LABEL_SPEED);
  P.setTextAlignment(z, PA_CENTER);
  P.setFont(z, FONT_METRO);
  P.setTextBuffer(z, labelBuffer[z-1]);
  P.setPause(z, LABEL_SPEED);
}

void setup() {
  Serial.begin(BAUD_RATE);
  P.begin(NUM_ZONES);
  beginAlert();
  beginLabel(ZONE_FOLLOW);
  beginLabel(ZONE_SUB);
  beginLabel(ZONE_CHEER);
}

void setAlert(EventType eventType, uint32_t num) {
  alertStage = ALERT_EVENT;
  // TODO set alertBuffer and alertMessage according to eventType
  P.setFont(ZONE_ALERTS, FONT_METRO);
  P.setTextBuffer(ZONE_ALERTS, alertBuffer);
  P.setTextEffect(ZONE_ALERTS, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
  P.displayReset(ZONE_ALERTS);
}

void setLabel(MatrixZone z, uint32_t num) {
  if (num == 0) {
    strncpy(labelBuffer[z-1], userBuffer, MAX_LABEL);
  } else {
    snprintf_P(labelBuffer[z-1], MAX_LABEL, PSTR("%s (%d)"), userBuffer, num);
  }

  if (P.getTextColumns(z, labelBuffer[z-1]) > SIZE_LABEL*8) {
    P.setTextEffect(z, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
  } else {
    P.setTextEffect(z, PA_PRINT, PA_NO_EFFECT);
  }
  P.displayReset(z);
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
              setAlert(eventType, num);
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
          switch (eventType) {
            case EVENT_FOLLOW:
              setLabel(ZONE_FOLLOW, num);
              break;
            case EVENT_SUB_NEW: case EVENT_SUB_RENEW: case EVENT_SUB_GIFT:
              setLabel(ZONE_SUB, num);
              break;
            case EVENT_CHEER:
              setLabel(ZONE_CHEER, num);
              break;
          }
        }
      }
      break;
    case INPUT_MESSAGE:
      if (rc != '\n') {
        messageBuffer[messageSize++] = rc;
      } else {
        messageBuffer[messageSize] = '\0';
        stage = INPUT_TARGET;
        setAlert(eventType, num);
      }
      break;
  }
}

void animateAlert() {
  if (P.getZoneStatus(ZONE_ALERTS)) {
    // TODO state management
    // TODO handle timed rewards
    // TODO send callbacks to the bot (how?), for TTS and queue unlock
  }
}

void animateLabel(MatrixZone z) {
  if (P.getZoneStatus(z)) {
    P.displayReset(z);
  }
}

void loop() {
  if (Serial.available()) {
    readSerial();
  }

  if (P.displayAnimate()) {
    animateAlert();
    animateLabel(ZONE_FOLLOW);
    animateLabel(ZONE_SUB);
    animateLabel(ZONE_CHEER);
  }
}
