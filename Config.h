#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <EEPROM.h>

#define HOT_ENGINE_PIN   D6
#define POOL_ENGINE_PIN  D7
#define BORDA_ENGINE_PIN D5
#define THERMISTOR_SWITCH_PIN  1 // TX
#define BUTTON_PIN             3 // RX
#define THERMISTOR_PIN A0

class Config {
  public:
    byte hotEngineTempDiff;
    byte hotEngineSecondsToRun;
    byte poolEngineStartHour;
    byte poolEngineStartMinute;
    byte poolEngineMinutesToRun;
    byte bordaEngineMinutesToRun;
    
    Config();
    void load();
    void save();
    void saveTest();
};

#endif
