#ifndef POOLLIGHT_H
#define POOLLIGHT_H

#include "ESPDateTime.h"
#include "Config.h"
#include "Status.h"
#include "FastLED.h"

#define NUM_LEDS 1

class PoolLight {

    private:
        Config *config;
        Status *status;
        CRGB led[NUM_LEDS];
        int clockPin, dataPin;
        long startTime;
        long lastRun;
        bool checkStart();
        bool checkStop();
    public:
        PoolLight(Status*, Config*);
        void turnOn();
        void turnOff();
        void begin();
        void loop();
};

#endif