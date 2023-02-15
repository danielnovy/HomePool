#include "PoolLight.h"

int counter = 0;

PoolLight::PoolLight(Status *status, Config* config) {
    this->config = config;
    this->status = status;
    this->algoBits = 0;
}

void PoolLight::begin() {
    pinMode(dataPin, OUTPUT);
    pinMode(clockPin, OUTPUT);
    FastLED.addLeds<P9813, P9813_DATA, P9813_CLOCK, RGB>(led, NUM_LEDS);
    this->turnOff();
}

void PoolLight::loop() {
  long inow = millis();
  if (inow - this->lastRun < 250) {
    return;
  }
  lastRun = inow;

  if (this->status->isPoolAlgoOn()) {
    executePoolLightAlgo();
    return;
  } 

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

void PoolLight::executePoolLightAlgo() {
  this->algo1();
}

void PoolLight::algo1() {
  if (algoBits == 0) {
    led[0] = CRGB(0, 255, 0); // green
  } else if (algoBits == 1) {
    led[0] = CRGB::Black;
  } else if (algoBits == 2) {
    led[0] = CRGB(255, 0, 0); // blue
  } else if (algoBits == 3) {
    led[0] = CRGB::Black;
  } else if (algoBits == 4) {
    led[0] = CRGB(0, 255, 0); // green
  } else if (algoBits == 5) {
    led[0] = CRGB::Black;
  } else if (algoBits == 6) {
    led[0] = CRGB(0, 255, 255); // yellow
  } else if (algoBits == 7) {
    led[0] = CRGB::Black;
  }
  FastLED.show();
  algoBits++;
  if (algoBits == 7) {
    algoBits = 0;
  }
}

void PoolLight::algoBrazil() {
  if (algoBits == 0) {
    //led[0] = CRGB(config->poolBlue, config->poolGreen, config->poolRed);
    led[0] = CRGB(255, 0, 0); // blue
  } else if (algoBits == 1) {
    led[0] = CRGB(0, 255, 0); // green
  } else {
    led[0] = CRGB(0, 255, 255); // yellow
  }
  FastLED.show();
  algoBits++;
  if (algoBits == 3) {
    algoBits = 0;
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

void PoolLight::turnAlgoOn() {
    this->status->setPoolAlgoOn(true);
}

void PoolLight::turnAlgoOff() {
    this->status->setPoolAlgoOn(false);
    led[0] = CRGB::Black;
    FastLED.show();
}