#ifndef MYLCD_H
#define MYLCD_H

#include <LiquidCrystal_I2C.h>
#include <ESPDateTime.h>

#include "Status.h"

#define SECONDS_TO_KEEP_BACKLIGHT 20

class MyLCD {
  public:
    bool isBacklightOn;
    MyLCD();
    void loop(bool, struct Status);
    void turnOn();
  private:
    long lastLcdBacklight;
    void printInfo(struct Status);
};

#endif
