#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>

#include "fonts.hpp"

#define HARDWARE_TYPE MD_MAX72XX::PAROLA_HW
#define MAX_DEVICES   54
#define CS_PIN        10

auto P = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);
String labels[3];

void setup() {
  Serial.begin(9600);
  P.begin(4);
  P.setZone(0,  0, 23);
  P.setZone(1, 24, 33);
  P.setZone(2, 34, 43);
  P.setZone(3, 44, 53);
  P.displayReset();
  P.displayZoneText(1, "Hello World!", PA_CENTER, P.getSpeed(), 1000, PA_SCROLL_DOWN, PA_SCROLL_DOWN);
}

void loop() {
  if (P.displayAnimate()) {
    P.getZoneStatus(0);
  }
}
