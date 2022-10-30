#ifndef CONFIG_C
#define CONFIG_C


#include "Config.h"

struct Config loadConfig() {
  struct Config result;
  EEPROM.begin(512);
  int addr = 0;
  result.hotEngineTempDiff = EEPROM.read(addr++);
  result.hotEngineSecondsToRun = EEPROM.read(addr++);
  result.poolEngineStartHour = EEPROM.read(addr++);
  result.poolEngineStartMinute = EEPROM.read(addr++);
  result.poolEngineMinutesToRun = EEPROM.read(addr++);

  return result;
}

void saveConfig(struct Config myConfig) {
  int addr = 0;
  EEPROM.write(addr++, myConfig.hotEngineTempDiff);
  EEPROM.write(addr++, myConfig.hotEngineSecondsToRun);
  EEPROM.write(addr++, myConfig.poolEngineStartHour);
  EEPROM.write(addr++, myConfig.poolEngineStartMinute);
  EEPROM.write(addr++, myConfig.poolEngineMinutesToRun);
  EEPROM.commit();
}

void saveTestConfig(struct Config myConfig) {
  
  EEPROM.begin(512);

  myConfig.hotEngineTempDiff = 10;
  myConfig.hotEngineSecondsToRun = 10;
  myConfig.poolEngineStartHour = 7;
  myConfig.poolEngineStartMinute = 0;
  myConfig.poolEngineMinutesToRun = 1;

  saveConfig(myConfig);

  myConfig.hotEngineTempDiff = 0;
  myConfig.hotEngineSecondsToRun = 0;
  myConfig.poolEngineStartHour = 0;
  myConfig.poolEngineStartMinute = 0;
  myConfig.poolEngineMinutesToRun = 0;
}


#endif
