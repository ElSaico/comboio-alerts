#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>

#include "fonts.h"

#define HARDWARE_TYPE MD_MAX72XX::PAROLA_HW
#define MAX_DEVICES   10
#define CS_PIN        10

auto P = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

void setup() {
  P.begin();
  P.setFont(fontIbm);
}

void loop() {
  if (P.displayAnimate())
    P.displayText("Hello World!", PA_CENTER, P.getSpeed(), 1000, PA_SCROLL_DOWN, PA_SCROLL_DOWN);
}
