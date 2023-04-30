#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <EEPROM.h>

#define THERMISTOR_PIN        A0
#define POOL_ENGINE_PIN       D5
#define HOT_ENGINE_PIN        D7
#define BORDA_ENGINE_PIN      D6
#define THERMISTOR_SWITCH_PIN D8
#define BUTTON_PIN            D0

#define POOL_RED              D3
#define POOL_GREEN            D9
#define POOL_BLUE             D10
#define P9813_DATA            D9
#define P9813_CLOCK           D10

// Aquecimento R2
// Borda R3
// Filtro R4
// Switch R1

class Config {
  public:
    byte hotEngineTempDiff;
    byte hotEngineSecondsToRun;
    byte poolEngineStartHour;
    byte poolEngineStartMinute;
    byte poolEngineHoursToRun;
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
