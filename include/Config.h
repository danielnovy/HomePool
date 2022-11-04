#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <EEPROM.h>

#define THERMISTOR_PIN   A0
#define HOT_ENGINE_PIN   D6
#define POOL_ENGINE_PIN  D7
#define BORDA_ENGINE_PIN D5
#define BUTTON_PIN       D0
#define POOL_RED         D3
#define POOL_GREEN       D9
#define POOL_BLUE        D10
#define THERMISTOR_SWITCH_PIN D8

class Config {
  public:
    byte hotEngineTempDiff;
    byte hotEngineSecondsToRun;
    byte poolEngineStartHour;
    byte poolEngineStartMinute;
    byte poolEngineMinutesToRun;
    byte bordaEngineMinutesToRun;
    byte poolLightStartHour;
    byte poolLightStartMinute;
    byte poolLightMinutesToRun;

    byte poolRed;
    byte poolGreen;
    byte poolBlue;
    
    Config();
    void load();
    void save();
    void saveTest();
};

#endif
