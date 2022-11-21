#include "MyLCD.h"

LiquidCrystal_I2C lcd(0x3F, 16, 2); // set the LCD address to 0x3F for a 16 chars and 2 line display

MyLCD::MyLCD(Status *status) {
  this->status = status;
}

void MyLCD::begin() {
  lcd.init();
  this->turnOn();
}

void MyLCD::loop() {
  if (isBacklightOn) {
    long inow = DateTime.now();
    if ((inow - this->lastLcdBacklight) > SECONDS_TO_KEEP_BACKLIGHT) {
      lcd.noBacklight();
      this->isBacklightOn = false;
    }
    if (this->status->infoChanged) {
      this->printInfo();
      this->status->infoChanged = false;
    }
  }
}

void MyLCD::turnOn() {
  lcd.backlight();
  this->lastLcdBacklight = DateTime.now();
  this->isBacklightOn = true;
}

void MyLCD::printInfo() {
  lcd.setCursor(0, 0);
  lcd.print("     ");
  lcd.setCursor(0, 0);
  lcd.print(String(this->status->getPoolTemperature()));
  lcd.setCursor(4, 0);
  lcd.print(" ");
  lcd.setCursor(5, 0);
  lcd.print((this->status->isHotEngineRunning())   ? "On  " : "Off");
  lcd.setCursor(9, 0);
  lcd.print((this->status->isPoolEngineRunning())  ? "On  " : "Off");
  lcd.setCursor(13, 0);
  lcd.print((this->status->isBordaEngineRunning()) ? "On  " : "Off");

  lcd.setCursor(0, 1);
  lcd.print("     ");
  lcd.setCursor(0, 1);
  lcd.print(String(this->status->getRoofTemperature()));
  lcd.setCursor(4, 1);
  lcd.print(" ");
  lcd.setCursor(5, 1);
  lcd.print("AQU ");
  lcd.setCursor(9, 1);
  lcd.print("FIL ");
  lcd.setCursor(13, 1);
  lcd.print("BOR ");
}
