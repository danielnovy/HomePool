#include "Button.h"

Button::Button(int pinNumber) {
  this->pinNumber = pinNumber;
}

void Button::begin() {
  pinMode(pinNumber, INPUT);
}

bool Button::isPressed() {
  long inow = millis();
  if (inow - this->lastPressed > 500L) {
    if (digitalRead(pinNumber) == LOW) {
      this->lastPressed = inow;
      return true;
    }
  }
  return false;
}
