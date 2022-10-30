#ifndef CONFIG_C
#define CONFIG_C

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
}

void Config::save() {
  int addr = 0;
  EEPROM.write(addr++, this->hotEngineTempDiff);
  EEPROM.write(addr++, this->hotEngineSecondsToRun);
  EEPROM.write(addr++, this->poolEngineStartHour);
  EEPROM.write(addr++, this->poolEngineStartMinute);
  EEPROM.write(addr++, this->poolEngineMinutesToRun);
  EEPROM.commit();
}

void Config::saveTest() {
  
  EEPROM.begin(512);

  this->hotEngineTempDiff = 10;
  this->hotEngineSecondsToRun = 10;
  this->poolEngineStartHour = 7;
  this->poolEngineStartMinute = 0;
  this->poolEngineMinutesToRun = 1;

  this->save();

  this->hotEngineTempDiff = 0;
  this->hotEngineSecondsToRun = 0;
  this->poolEngineStartHour = 0;
  this->poolEngineStartMinute = 0;
  this->poolEngineMinutesToRun = 0;
}


#endif
