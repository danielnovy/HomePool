#ifndef GENERICENGINE_H
#define GENERICENGINE_H

#include <Arduino.h> 
#include <ESPDateTime.h>
#include "Config.h"

class GenericEngine {
  
  public:
    bool running;
    GenericEngine(int pinNumber, bool pool);
    bool loop();
    void begin(Config *myConfig);
    void start();
    void stop();
    
  private:
    Config *myConfig;
    int pinNumber;
    long startTime;
    long lastRun;
    bool pool;
    bool checkStart();
    bool checkStop();
};

#endif
