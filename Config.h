#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <EEPROM.h>

struct Config {
  byte hotEngineTempDiff;
  byte hotEngineSecondsToRun;

  byte poolEngineStartHour;
  byte poolEngineStartMinute;
  byte poolEngineMinutesToRun;
};

struct Config loadConfig();
void saveConfig(Config);

#endif
