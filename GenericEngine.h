#ifndef GENERICENGINE_H
#define GENERICENGINE_H

#include <Arduino.h> 
#include <ESPDateTime.h>
#include "Config.h"
#include "MyLCD.h"
#include "Status.h"

class GenericEngine {
  
  public:
    bool running;
    GenericEngine(Status *status, Config *myConfig, int pinNumber, bool pool);
    void loop();
    void begin();
    void start();
    void stop();
    
  private:
    Config *myConfig;
    Status *status;
    int pinNumber;
    long startTime;
    long lastRun;
    bool pool;
    bool checkStart();
    bool checkStop();
};

#endif
