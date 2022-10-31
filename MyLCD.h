#ifndef MYLCD_H
#define MYLCD_H

#include <LiquidCrystal_I2C.h>
#include <ESPDateTime.h>

#include "Config.h"
#include "GenericEngine.h"
#include "HotEngine.h"

#define SECONDS_TO_KEEP_BACKLIGHT 20

class MyLCD {
  public:
    bool isBacklightOn;
    MyLCD(GenericEngine* poolEngine, GenericEngine* bordaEngine, HotEngine* hotEngine);
    void turnOn();
    void updateScreen(bool);
  private:
    GenericEngine *poolEngine;
    GenericEngine *bordaEngine;
    HotEngine *hotEngine;
    long lastLcdBacklight;
    void printInfo();
};

#endif
