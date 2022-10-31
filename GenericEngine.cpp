#include "GenericEngine.h"

GenericEngine::GenericEngine(Status *status, Config *myConfig, int pinNumber, bool pool) {
  this->myConfig = myConfig;
  this->status = status;
  this->pinNumber = pinNumber;
  this->pool = pool;
}

void GenericEngine::begin() {
  pinMode(pinNumber,  OUTPUT);
  digitalWrite(pinNumber,  HIGH);
}

void GenericEngine::loop() {
  long inow = DateTime.now();
  if (inow - this->lastRun < 5) {
    // only run once each 5s
    return;
  }
  lastRun = inow;
  if (this->running) {
    if (checkStop()) {
      this->stop();
    }
  } else {
    if (checkStart()) {
      this->start();
    }
  }
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
  if (pool) this->status->setPoolEngineRunning(true);
  else this->status->setBordaEngineRunning(true);
}

void GenericEngine::stop() {
  digitalWrite(pinNumber, HIGH);
  this->running = false;
  if (pool) this->status->setPoolEngineRunning(false);
  else this->status->setBordaEngineRunning(false);
}
