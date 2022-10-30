#include "MyLCD.h"

LiquidCrystal_I2C lcd(0x3F,16,2);  // set the LCD address to 0x3F for a 16 chars and 2 line display

MyLCD::MyLCD() {
  lcd.init();
  lcd.backlight();
  this->lastLcdBacklight = DateTime.now();
  this->isBacklightOn = true;
}

void MyLCD::loop(bool infoChanged, struct Status currStatus) {
  if (infoChanged) {
    this->printInfo(currStatus);
  }
  long inow = DateTime.now();
  if ((inow - this->lastLcdBacklight) > SECONDS_TO_KEEP_BACKLIGHT) {
    lcd.noBacklight();
    this->isBacklightOn = false;
  }
}

void MyLCD::turnOn() {
  lcd.backlight();
  this->lastLcdBacklight = DateTime.now();
  this->isBacklightOn = true;
}

void MyLCD::printInfo(struct Status currStatus) {
  lcd.setCursor(0, 0);
  lcd.print("     ");
  lcd.setCursor(0, 0);
  lcd.print(currStatus.roofTemp);
  lcd.setCursor(4, 0);
  lcd.print(" ");
  lcd.setCursor(4, 0);
  lcd.print(" ");
  lcd.setCursor(5, 0);
  lcd.print((currStatus.hotEngineRunning)   ? "On " : "Off");
  lcd.setCursor(9, 0);
  lcd.print((currStatus.poolEngineRunning)  ? "On " : "Off");
  lcd.setCursor(13, 0);
  lcd.print((currStatus.bordaEngineRunning) ? "On " : "Off");

  lcd.setCursor(0, 1);
  lcd.print("     ");
  lcd.setCursor(0, 1);
  lcd.print(currStatus.poolTemp);
  lcd.setCursor(4, 1);
  lcd.print(" ");
  lcd.setCursor(4, 1);
  lcd.setCursor(5, 1);
  lcd.print("AQU");
  lcd.setCursor(9, 1);
  lcd.print("FIL");
  lcd.setCursor(13, 1);
  lcd.print("BOR");
}
