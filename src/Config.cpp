#include "Config.h"

Config::Config() {
}

void Config::load() {
  EEPROM.begin(512);
  int addr = 0;
  this->hotEngineTempDiff = EEPROM.read(addr++);
  this->hotEngineSecondsToRun = EEPROM.read(addr++);
  this->poolEngineStartHour = EEPROM.read(addr++);
  this->poolEngineStartMinute = EEPROM.read(addr++);
  this->poolEngineMinutesToRun = EEPROM.read(addr++);
  this->bordaEngineMinutesToRun = EEPROM.read(addr++);
  //this->poolLightStartHour = EEPROM.read(addr++);
  //this->poolLightStartMinute = EEPROM.read(addr++);
  //this->poolLightMinutesToRun = EEPROM.read(addr++);
  //this->poolRed = EEPROM.read(addr++);
  //this->poolGreen = EEPROM.read(addr++);
  //this->poolBlue = EEPROM.read(addr++);
}

void Config::save() {
  int addr = 0;
  EEPROM.write(addr++, this->hotEngineTempDiff);
  EEPROM.write(addr++, this->hotEngineSecondsToRun);
  EEPROM.write(addr++, this->poolEngineStartHour);
  EEPROM.write(addr++, this->poolEngineStartMinute);
  EEPROM.write(addr++, this->poolEngineMinutesToRun);
  EEPROM.write(addr++, this->bordaEngineMinutesToRun);
  EEPROM.write(addr++, this->poolLightStartHour);
  EEPROM.write(addr++, this->poolLightStartMinute);
  EEPROM.write(addr++, this->poolLightMinutesToRun);
  EEPROM.write(addr++, this->poolRed);
  EEPROM.write(addr++, this->poolGreen);
  EEPROM.write(addr++, this->poolBlue);
  EEPROM.commit();
}