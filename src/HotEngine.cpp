#include "HotEngine.h"

float roofTemps[10];
int roofTempsIndex = 0;
float poolTemps[10];
int poolTempsIndex = 0;

HotEngine::HotEngine(Status *status, Config *myConfig, int pinNumber, int switchPinNumber) {
  this->status = status;
  this->myConfig = myConfig;
  this->pinNumber = pinNumber;
  this->switchPinNumber = switchPinNumber;
  this->thermistor = new Thermistor(THERMISTOR_PIN);
  this->pool = false;
}

void HotEngine::begin() {
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
  digitalWrite(switchPinNumber, HIGH); // Chaveia para sensor do telhado (T1)

  float temp = this->thermistor->computeTemperature(this->lastPoolRead);
  poolTemps[poolTempsIndex++] = temp;
  if (poolTempsIndex >= 10) poolTempsIndex = 0;
  float sum = 0;
  int total = 0;
  for (int i = 0; i < 10; i++) {
    if (poolTemps[i] > 0) {
      sum += poolTemps[i];
      total++;
    }
  }
  if (total > 0) temp = sum / total;

  this->poolTemperature = temp;
  this->status->setPoolTemperature(this->poolTemperature);
}

void HotEngine::measureRoofTemperature() {
  this->lastRoofRead = this->thermistor->readSensor();
  digitalWrite(switchPinNumber, LOW); // chaveia para sensor da piscina (T2)

  float temp = this->thermistor->computeTemperature(this->lastRoofRead);
  roofTemps[roofTempsIndex++] = temp;
  if (roofTempsIndex >= 10) roofTempsIndex = 0;
  float sum = 0;
  int total = 0;
  for (int i = 0; i < 10; i++) {
    if (roofTemps[i] > 0) {
      sum += roofTemps[i];
      total++;
    }
  }
  if (total > 0) temp = sum / total;

  this->roofTemperature = temp;
  this->status->setRoofTemperature(this->roofTemperature);
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
  if (diff > this->myConfig->hotEngineSecondsToRun) {
    return true;
  }
  return false;
}

void HotEngine::start() {
  this->startTime = DateTime.now();
  digitalWrite(pinNumber, LOW);
  this->running = true;
  this->status->setHotEngineRunning(true);
}

void HotEngine::stop() {
  digitalWrite(pinNumber, HIGH);
  this->running = false;
  this->status->setHotEngineRunning(false);
}
