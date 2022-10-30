#ifndef BUTTON_H
#define BUTTON_H

#include <Arduino.h>

class Button {
  public:
    Button(int pinNumber);
    void begin();
    bool isPressed();
  private:
    int pinNumber;
    long lastPressed;
};

#endif
