#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <EEPROM.h>

class Config {
  public:
    byte hotEngineTempDiff;
    byte hotEngineSecondsToRun;
    byte poolEngineStartHour;
    byte poolEngineStartMinute;
    byte poolEngineMinutesToRun;
    
    Config();
    void load();
    void save();
    void saveTest();
};


#endif
