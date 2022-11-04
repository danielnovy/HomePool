#ifndef HOTENGINE_H
#define HOTENGINE_H

#include <Arduino.h> 
#include <ESPDateTime.h>
#include "Config.h"
#include "Thermistor.h"
#include "Status.h"

class HotEngine {
  
  public:
    bool running;
    float roofTemperature, poolTemperature;
    int lastRoofRead, lastPoolRead;
    HotEngine(Status *status, Config *myConfig, int pinNumber, int switchPinNumber);
    bool loop();
    void begin();
    void start();
    void stop();
    
  private:
    Config *myConfig;
    Status *status;
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
