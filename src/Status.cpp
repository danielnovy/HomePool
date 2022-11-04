#include "Status.h"

void Status::setPoolEngineRunning(bool v) {
  this->poolEngineRunning = v;
  this->infoChanged = true;
}
bool Status::isPoolEngineRunning() {
  return this->poolEngineRunning;
}
void Status::setBordaEngineRunning(bool v) {
  this->bordaEngineRunning = v;
  this->infoChanged = true;
}
bool Status::isBordaEngineRunning() {
  return this->bordaEngineRunning;
  
}
void Status::setHotEngineRunning(bool v) {
  this->hotEngineRunning = v;
  this->infoChanged = true;
}
bool Status::isHotEngineRunning() {
  return this->hotEngineRunning;
}
void  Status::setRoofTemperature(float v) {
  this->roofTemperature = v;
  this->infoChanged = true;
}
float Status::getRoofTemperature() {
  return this->roofTemperature;
}
void  Status::setPoolTemperature(float v) {
  this->poolTemperature = v;
  this->infoChanged = true;
}
float Status::getPoolTemperature() {
  return this->poolTemperature;
}
