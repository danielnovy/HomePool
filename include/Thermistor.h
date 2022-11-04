#ifndef THERMISTOR_H
#define THERMISTOR_H

#include <Arduino.h>

class Thermistor {
  public:
    Thermistor(int pinNumber);
    int readSensor();
    float computeTemperature(int aread);
  private:
    int pinNumber;
};

#endif
