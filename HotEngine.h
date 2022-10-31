#ifndef HOTENGINE_H
#define HOTENGINE_H

#include <Arduino.h> 
#include <ESPDateTime.h>
#include "Config.h"
#include "Thermistor.h"

class HotEngine {
  
  public:
    bool running;
    float roofTemperature, poolTemperature;
    int lastRoofRead, lastPoolRead;
    HotEngine(int pinNumber, int switchPinNumber);
    bool loop();
    void begin(Config *myConfig);
    void start();
    void stop();
    
  private:
    Config *myConfig;
    Thermistor *thermistor;
    int pinNumber, switchPinNumber;
    long startTime;
    long lastRun;
    bool pool;
    void measureRoofTemperature();
    void measurePoolTemperature();
    bool checkStart();
    bool checkStop();
};

#endif
