#ifndef MYLCD_H
#define MYLCD_H

#include <LiquidCrystal_I2C.h>
#include <ESPDateTime.h>

#include "Config.h"
#include "Status.h"

#define SECONDS_TO_KEEP_BACKLIGHT 20

class MyLCD {
  public:
    bool isBacklightOn;
    MyLCD(Status *status);
    void turnOn();
    void loop();
    void printInfo();
    void begin();
  private:
    Status *status;
    long lastLcdBacklight;
};

#endif
