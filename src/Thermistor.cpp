#include "Thermistor.h"

const int reads[]    = {275, 403,  504,  511,  527,  539,  555,  564,  577,  600,  610,  629,  651,  686,  703,  716,  756,  779,  804,  839,  857,  872,  888,  905};
const double temps[] = {  3,  14, 22.3,   23, 24.4, 25.1, 26.5, 27.5, 28.5, 30.5, 31.5, 32.8, 34.5, 38.5, 40.0, 41.1, 44.9, 47.6, 50.2, 55.0, 57.6, 60.0, 62.5, 64.5}; 

Thermistor::Thermistor(int pinNumber) {
  this->pinNumber = pinNumber;
  pinMode(pinNumber, INPUT);
}

int Thermistor::readSensor() {
  int times = 10;
  int result = 0;
  for (int i = 0; i < times; i++) {
    result += analogRead(this->pinNumber);
    delay(10);
  }
  return (result / times);
}

float Thermistor::computeTemperature(int aread) {
  int slot = -1;
  int reads_size = sizeof(reads);
  for (int i = 1; i < reads_size - 1; i++) {
    if (aread >= reads[i-1] && aread < reads[i]) {
      slot = i - 1;
      break;
    }
  }
  if (slot == -1) return 0;
  double tempdiff = temps[slot+1] - temps[slot];
  int readdiff = reads[slot+1] - reads[slot];
  double increment = tempdiff / readdiff;
  return (increment * (aread - reads[slot]) + temps[slot]);
}
