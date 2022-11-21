#include "PoolLight.h"

PoolLight::PoolLight(Status *status, Config* config) {
    this->config = config;
    this->status = status;
}

void PoolLight::begin() {
    pinMode(dataPin, OUTPUT);
    pinMode(clockPin, OUTPUT);
    FastLED.addLeds<P9813, P9813_DATA, P9813_CLOCK, RGB>(led, NUM_LEDS);
    this->turnOff();
}

void PoolLight::loop() {
  long inow = DateTime.now();
  if (inow - this->lastRun < 5) {
    // only run once each 5s
    return;
  }
  lastRun = inow;

  if (this->status->isPoolLightOn()) {
    if (checkStop()) {
      this->turnOff();
    }
  } else {
    if (checkStart()) {
      this->turnOn();
    }
  }
}

bool PoolLight::checkStart() {
  int hour = DateTime.format("%H").toInt();
  int min  = DateTime.format("%M").toInt();
  bool matchHour = hour == this->config->poolLightStartHour;
  bool matchMin  = min == this->config->poolLightStartMinute;
//  if (hour == 0 && min == 0) {
//    ESP.restart();
//  }
  return matchHour && matchMin;
}

bool PoolLight::checkStop() {
  long inow = DateTime.now();
  long diff = inow - this->startTime;
  if (diff > (this->config->poolLightMinutesToRun * 60 * 60)) {
    return true;
  }
  return false;
}

void PoolLight::turnOn() {
    this->status->setPoolLightOn(true);
    this->startTime = DateTime.now();
    //led[0] = CRGB(config->poolRed, config->poolGreen, config->poolBlue);
    // Instalei os fios invertidos, por isso não tá RGB abaixo.
    led[0] = CRGB(config->poolBlue, config->poolGreen, config->poolRed);
    FastLED.show();
}

void PoolLight::turnOff() {
    this->status->setPoolLightOn(false);
    led[0] = CRGB::Black;
    FastLED.show();
}