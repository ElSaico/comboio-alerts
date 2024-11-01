#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>

#include "fonts.hpp"

#define BAUD_RATE      9600
#define CS_PIN         10

#define NUM_ZONES      4
#define NUM_MODULES    54
#define NUM_EVENTS     (sizeof(EVENTS) / sizeof(EVENTS[0])) // easier than keeping track of array size

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

enum inputStage_t : uint8_t { INPUT_TARGET, INPUT_EVENT, INPUT_NUMBER, INPUT_USER, INPUT_MESSAGE };
enum zone_t : uint8_t { ZONE_ALERTS, ZONE_FOLLOW, ZONE_SUB, ZONE_CHEER, ZONE_INVALID };
enum alertStage_t : uint8_t { ALERT_IDLE, ALERT_EVENT, ALERT_USER, ALERT_MESSAGE };
enum eventCode_t : char {
  EVENT_FOLLOW = 'F',
  EVENT_SUB_NEW = 'S',
  EVENT_SUB_RENEW = 's',
  EVENT_SUB_GIFT = 'G',
  EVENT_CHEER = 'C',
  EVENT_RAID = 'R',
  EVENT_DONATE = 'D',
  EVENT_SHOUTOUT = 'O',
};
struct event_t {
  eventCode_t code;
  zone_t zone;
  const char alert[48];
  const char alertSingular[16];
  const char alertPlural[16];
  bool alertMessage;
};

/** WARNING: ensure all strings are on Parola's native encoding (ISO-8859-1) */
const event_t EVENTS[] PROGMEM = {
  { EVENT_FOLLOW, ZONE_FOLLOW, "Novo passageiro no Comboio", "", "", false },
  { EVENT_SUB_NEW, ZONE_SUB, "Nova aquisição de passe", "", "", false /* IIRC? */ },
  { EVENT_SUB_RENEW, ZONE_SUB, "Renovação de passe, totalizando %d meses", "", "", true },
  { EVENT_SUB_GIFT, ZONE_SUB, "%d %s para o Comboio", "passe doado", "passes doados", false },
  { EVENT_CHEER, ZONE_CHEER, "%d %s para o Comboio", "bit enviado", "bits enviados", true },
  { EVENT_RAID, ZONE_INVALID, "Embarque de uma raid com %d %s", "pessoa", "pessoas", true },
  { EVENT_DONATE, ZONE_INVALID, "Pix de %s enviado para o Comboio", "", "", true },
  { EVENT_SHOUTOUT, ZONE_INVALID, "O Comboio do Saico recomenda este canal", "", "", true },
};

auto P = MD_Parola(MD_MAX72XX::PAROLA_HW, CS_PIN, NUM_MODULES);
char userBuffer[MAX_LABEL];
char messageBuffer[MAX_MESSAGE];
char labelBuffer[NUM_ZONES-1][MAX_LABEL];
char alertBuffer[MAX_ALERT];
bool alertMessage;
auto alertStage = ALERT_IDLE;
const event_t* event;

void beginAlert() {
  P.setZone(ZONE_ALERTS, 0, SIZE_ALERTS-1);
  P.setSpeed(ZONE_ALERTS, LABEL_SPEED);
  P.setTextAlignment(ZONE_ALERTS, PA_CENTER);
  P.setPause(ZONE_ALERTS, 0);
}

void beginLabel(zone_t z) {
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

void setAlert(uint32_t num) {
  alertStage = ALERT_EVENT;
  if (pgm_read_byte(&event->alertPlural[0])) {
    auto numSuffix = (const char*) pgm_read_word(num == 1 ? &event->alertSingular : &event->alertPlural);
    snprintf_P(alertBuffer, MAX_ALERT, event->alert, num, numSuffix);
  } else {
    snprintf_P(alertBuffer, MAX_ALERT, event->alert, num);
  }
  P.setFont(ZONE_ALERTS, FONT_METRO);
  P.setTextBuffer(ZONE_ALERTS, alertBuffer);
  P.setTextEffect(ZONE_ALERTS, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
  P.displayReset(ZONE_ALERTS);
}

void setLabel(uint32_t num) {
  auto z = static_cast<zone_t> pgm_read_byte(&event->zone);
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
  static inputStage_t stage = INPUT_TARGET;
  static char target;
  static uint32_t num; // rather optimistic to support a raid of over 65535, but it's only 2 extra bytes
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
      for (const auto& e : EVENTS) {
        if (rc == pgm_read_byte(&e.code)) {
          event = &e;
          break;
        }
      }
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
        if (event == nullptr) {
          stage = INPUT_TARGET;
        } else if (target == TARGET_ALERTS) {
          if (pgm_read_byte(&event->alertMessage)) {
            stage = INPUT_MESSAGE;
            messageSize = 0;
          } else {
            stage = INPUT_TARGET;
            setAlert(num);
          }
        } else {
          stage = INPUT_TARGET;
          setLabel(num);
        }
      }
      break;
    case INPUT_MESSAGE:
      if (rc != '\n') {
        messageBuffer[messageSize++] = rc;
      } else {
        messageBuffer[messageSize] = '\0';
        stage = INPUT_TARGET;
        setAlert(num);
      }
      break;
  }
}

void animateAlert() {
  if (P.getZoneStatus(ZONE_ALERTS)) {
    switch (alertStage) {
      case ALERT_EVENT:
        alertStage = ALERT_USER;
        // TODO slow down
        P.setTextBuffer(ZONE_ALERTS, userBuffer);
        P.setTextEffect(ZONE_ALERTS, PA_GROW_DOWN, PA_GROW_UP);
        P.displayReset(ZONE_ALERTS);
        break;
      case ALERT_USER:
        if (pgm_read_byte(&event->alertMessage)) {
          alertStage = ALERT_MESSAGE;
          P.setFont(ZONE_ALERTS, FONT_IBM);
          P.setTextBuffer(ZONE_ALERTS, messageBuffer);
          P.setTextEffect(ZONE_ALERTS, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
          P.displayReset(ZONE_ALERTS);
        } else {
          alertStage = ALERT_IDLE;
        }
        break;
      case ALERT_MESSAGE:
        alertStage = ALERT_IDLE;
        break;
    }
    // TODO handle timed rewards
    // TODO send callbacks to the bot (how?), for TTS and queue unlock
  }
}

void animateLabel(zone_t z) {
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
