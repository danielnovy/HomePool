#include "HotEngine.h"

HotEngine::HotEngine(int pinNumber, int switchPinNumber) {
  this->pinNumber = pinNumber;
  this->switchPinNumber = switchPinNumber;
  this->thermistor = new Thermistor(THERMISTOR_PIN);
  this->pool = false;
}

void HotEngine::begin(Config *myConfig) {
  this->myConfig = myConfig;
  pinMode(pinNumber,  OUTPUT);
  digitalWrite(pinNumber,  HIGH);

  pinMode(switchPinNumber, OUTPUT);
  digitalWrite(switchPinNumber, LOW); // Comeca lendo sensor do telhado
}

bool HotEngine::loop() {
  long inow = DateTime.now();
  if (inow - this->lastRun < 5) {
    // only run once each 5s
    return false;
  }
  lastRun = inow;

  if (pool) {
    measurePoolTemperature();
  } else {
    measureRoofTemperature();
  }
  pool = !pool;
  
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

void HotEngine::measurePoolTemperature() {
  this->lastPoolRead = this->thermistor->readSensor();
  this->poolTemperature = this->thermistor->computeTemperature(this->lastPoolRead); 
  digitalWrite(switchPinNumber, LOW); // Chaveia para sensor do telhado (T1)
}

void HotEngine::measureRoofTemperature() {
  this->lastRoofRead = this->thermistor->readSensor();
  this->roofTemperature = this->thermistor->computeTemperature(this->lastRoofRead); 
  digitalWrite(switchPinNumber, HIGH); // chaveia para sensor da piscina (T2)
}

bool HotEngine::checkStart() {
  if (this->roofTemperature == 0 || this->poolTemperature == 0) {
    return false;
  }
  return this->roofTemperature - this->poolTemperature >= myConfig->hotEngineTempDiff;
}

bool HotEngine::checkStop() {
  long inow = DateTime.now();
  long diff = inow - this->startTime;
  if (diff > myConfig->hotEngineSecondsToRun) {
    return true;
  }
  return false;
}

void HotEngine::start() {
  this->startTime = DateTime.now();
  digitalWrite(pinNumber, LOW);
  this->running = true;
}

void HotEngine::stop() {
  digitalWrite(pinNumber, HIGH);
  this->running = false;
}
