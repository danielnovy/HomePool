#ifndef SETUP_H
#define SETUP_H

#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>
#include <ESPDateTime.h>

#define HOT_ENGINE_PIN   D6
#define POOL_ENGINE_PIN  D7
#define BORDA_ENGINE_PIN D5
#define SENSOR_SWITCH    1 // TX
#define BUTTON_PIN       3 // RX

void setupAll();

#endif
