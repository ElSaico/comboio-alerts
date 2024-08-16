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
uint8_t flushZones = 0;
String labels[NUM_ZONES-1];

void setup() {
  Serial.begin(9600);
  P.begin(NUM_ZONES);
  for (uint8_t i = 0; i < NUM_ZONES; i++) {
    P.setZone(i, ZONES[i], ZONES[i+1]-1);
  }
}

void loop() {
  if (P.displayAnimate()) {
    for (uint8_t i = 1; i < NUM_ZONES; i++) {
      if (flushZones & (1 << i)) {
        P.displayReset(i);
      }
    }
  }
}
