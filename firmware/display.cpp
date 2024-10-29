#include "display.hpp"
#include "fonts.hpp"

LabelDisplay::LabelDisplay(MD_Parola *P, MatrixZone zone): _P(P), _z(zone) {}

void LabelDisplay::begin() {
  _P->setZone(_z, ZONE_BEGIN(_z), ZONE_END(_z));
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
