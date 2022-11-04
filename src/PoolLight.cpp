#include "PoolLight.h"

PoolLight::PoolLight(Status *status, Config* config, int rPin, int gPin, int bPin) {
    this->config = config;
    this->status = status;
    this->rPin = rPin;
    this->gPin = gPin;
    this->bPin = bPin;
}

void PoolLight::begin() {
    pinMode(rPin, OUTPUT);
    pinMode(gPin, OUTPUT);
    pinMode(bPin, OUTPUT);

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
  bool matchHour = DateTime.format("%H").toInt() == this->config->poolLightStartHour;
  bool matchMin  = DateTime.format("%M").toInt() == this->config->poolLightStartMinute;
  return matchHour && matchMin;
}

bool PoolLight::checkStop() {
  long inow = DateTime.now();
  long diff = inow - this->startTime;
  if (diff > this->config->poolLightMinutesToRun) {
    return true;
  }
  return false;
}


void PoolLight::turnOn() {
    analogWrite(rPin, config->poolRed);
    analogWrite(gPin, config->poolGreen);
    analogWrite(bPin, config->poolBlue);
    this->status->setPoolLightOn(true);
    this->startTime = DateTime.now();
}

void PoolLight::turnOff() {
    analogWrite(rPin, 0);
    analogWrite(gPin, 0);
    analogWrite(bPin, 0);
    this->status->setPoolLightOn(false);
}