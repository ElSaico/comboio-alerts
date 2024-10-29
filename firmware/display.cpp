#include "display.hpp"
#include "fonts.hpp"

LabelDisplay::LabelDisplay(MD_Parola *P, MatrixZone zone): _P(P), _z(zone) {}

void LabelDisplay::begin() {
  _P->setZone(_z, LABEL_START(_z), LABEL_END(_z));
  _P->setSpeed(_z, LABEL_SPEED);
  _P->setTextAlignment(_z, PA_CENTER);
  _P->setFont(_z, FONT_METRO);
  _P->setTextBuffer(_z, buffer);
  _P->setPause(_z, LABEL_SPEED);
}

void LabelDisplay::set(const char *user, uint32_t num) {
  if (num == 0) {
    strncpy(buffer, user, MAX_LABEL);
  } else {
    snprintf_P(buffer, MAX_LABEL, PSTR("%s (%d)"), user, num);
  }

  if (_P->getTextColumns(_z, buffer) > SIZE_LABEL*8) {
    _P->setTextEffect(_z, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
  } else {
    _P->setTextEffect(_z, PA_PRINT, PA_NO_EFFECT);
  }
  _P->displayReset(_z);
}

void LabelDisplay::animate() {
  if (_P->getZoneStatus(_z)) {
    _P->displayReset(_z);
  }
}

AlertDisplay::AlertDisplay(MD_Parola *P, char *user, char *message):
  _P(P), _user(user), _message(message) {}

void AlertDisplay::begin() {
  _P->setZone(ZONE_ALERTS, 0, SIZE_ALERTS-1);
  _P->setSpeed(ZONE_ALERTS, LABEL_SPEED);
  _P->setTextAlignment(ZONE_ALERTS, PA_CENTER);
  _P->setPause(ZONE_ALERTS, 0);
}

void AlertDisplay::set(EventType eventType, uint32_t num) {
  stage = ALERT_EVENT;
  // TODO set alert and withMessage according to eventType
  _P->setFont(ZONE_ALERTS, FONT_METRO);
  _P->setTextBuffer(ZONE_ALERTS, alert);
  _P->setTextEffect(ZONE_ALERTS, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
  _P->displayReset(ZONE_ALERTS);
}

void AlertDisplay::animate() {
  if (_P->getZoneStatus(ZONE_ALERTS)) {
    // TODO state management
    // send callbacks to the bot (how?), for TTS and queue unlock
  }
}
