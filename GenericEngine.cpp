#include "GenericEngine.h"

GenericEngine::GenericEngine(int pinNumber, bool pool) {
  this->pinNumber = pinNumber;
  this->pool = pool;
}

void GenericEngine::begin(Config *myConfig) {
  pinMode(pinNumber,  OUTPUT);
  digitalWrite(pinNumber,  HIGH);
  this->myConfig = myConfig;
}

bool GenericEngine::loop() {
  long inow = DateTime.now();
  if (inow - this->lastRun < 5) {
    // only run once each 5s
    return false;
  }
  lastRun = inow;
  if (this->running) {
    if (checkStop()) {
      this->stop();
      return true;
    }
  } else {
    if (checkStart()) {
      this->start();
      return true;
    }
  }
  return false;
}

bool GenericEngine::checkStart() {
  bool matchHour = DateTime.format("%H").toInt() == this->myConfig->poolEngineStartHour;
  bool matchMin  = DateTime.format("%M").toInt() == this->myConfig->poolEngineStartMinute;
  return matchHour && matchMin;
}

bool GenericEngine::checkStop() {
  long inow = DateTime.now();
  long diff = inow - this->startTime;
  byte minutes = (pool) ? this->myConfig->poolEngineMinutesToRun : this->myConfig->bordaEngineMinutesToRun;
  if (diff > (minutes * 60L)) { 
    return true;
  }
  return false;
}

void GenericEngine::start() {
  this->startTime = DateTime.now();
  digitalWrite(pinNumber, LOW);
  this->running = true;
}

void GenericEngine::stop() {
  digitalWrite(pinNumber, HIGH);
  this->running = false;
}
