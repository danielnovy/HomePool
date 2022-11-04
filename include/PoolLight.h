#ifndef POOLLIGHT_H
#define POOLLIGHT_H

#include "ESPDateTime.h"
#include "Config.h"
#include "Status.h"

class PoolLight {

    private:
        Config *config;
        Status *status;
        int rPin, gPin, bPin;
        long startTime;
        long lastRun;
        bool checkStart();
        bool checkStop();
    public:
        PoolLight(Status*, Config*, int r, int g, int b);
        void turnOn();
        void turnOff();
        void begin();
        void loop();
};

#endif