#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>

#include "fonts.hpp"

#define HARDWARE_TYPE MD_MAX72XX::PAROLA_HW
#define MAX_DEVICES   54
#define CS_PIN        10
#define NUM_ZONES     4

auto P = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);
uint8_t ZONES[] = { 0, 24, 34, 44, 54 };
String labels[NUM_ZONES-1];

void setup() {
  Serial.begin(9600);
  P.begin(NUM_ZONES);
  for (uint8_t i = 0; i < NUM_ZONES; i++) {
    P.setZone(i, ZONES[i], ZONES[i+1]-1);
  }
}

void loop() {
  static uint8_t zone = 0;

  P.displayZoneText(zone, "Hello World!", PA_LEFT, P.getSpeed(), 1000, PA_SCROLL_DOWN, PA_SCROLL_DOWN);
  while (!P.getZoneStatus(zone)) {
    P.displayAnimate();
  }

  zone = (zone + 1) % NUM_ZONES;
}
