//#include <ESP8266WiFi.h>
//#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <uri/UriBraces.h>


#include "Config.h"
#include "Setup.h"
#include "MyLCD.h"
#include "Button.h"
#include "GenericEngine.h"
#include "HotEngine.h"
#include "WebServer.h"
#include "Status.h"

// Conexoes:
// Modulo 4 reles: d7, d6, d5 e tx
// LCD: d2, d1
// Button: rx, gnd
// Thermistor: a0 e gnd
// Resistor 10k: 3.3v -> a0


// PLACA WeMos D1 R1 ESP8266
// Pinos que posso usar como INPUT D5, D6, D7 (d8 nao, d4 nao, d3 nao, d2 nao, d1 inteferiu na saida serial!, d0 nao)
// D1: i2c SCL
// D2: i2c SDA
// D3 (GPIO0): pull low during boot for flash mode. Naturally pulled up (10k).
//    Usar o D3 como output sacaneiou meu LCD
// D4 (GPIO2): pulled up (10k), internal led (active => LOW). Debug serial pin. OUTPUT ONLY.
//    Usar o D4 como output sacaneiou meu LCD
// D8 (GPIO15): pulled to ground.
// RX (GPIO3): INPUT ONLY (FUNCTION_3)
// TX (GPIO1): OUTPUT ONLY (FUNCTION_3)

// No meu caso, D1 e D2 jah foram pro saco por causa do display. Vou usar RX para um INPUT e TX para um OUTPUT.
// Faltam 2 INPUTs e 3 OUTPUTs.

Config *myConfig = new Config();
Button *button   = new Button(BUTTON_PIN);
Status *status   = new Status();

GenericEngine *poolEngine  = new GenericEngine(status, myConfig, POOL_ENGINE_PIN, true);
GenericEngine *bordaEngine = new GenericEngine(status, myConfig, BORDA_ENGINE_PIN, false);
HotEngine     *hotEngine   = new HotEngine(status, myConfig, HOT_ENGINE_PIN, THERMISTOR_SWITCH_PIN);
WebServer     *webServer   = new WebServer(status, myConfig, poolEngine, bordaEngine, hotEngine);

MyLCD  *mylcd = new MyLCD(status);

void setup() {
  //myConfig->saveTest();
  myConfig->load();
  setupAll();
  button->begin();
  poolEngine->begin();
  bordaEngine->begin();
  hotEngine->begin();
  webServer->begin();
}

void loop() {
  ArduinoOTA.handle();
  webServer->loop();
  mylcd->loop();
  poolEngine->loop();
  bordaEngine->loop();
  hotEngine->loop();
  checkButtonPressed();
}

void checkButtonPressed() {
  if (button->isPressed()) {
    if (mylcd->isBacklightOn) {
      if (poolEngine->running) {
        poolEngine->stop();
        status->setPoolEngineRunning(false);
      } else {
        poolEngine->start();
        status->setPoolEngineRunning(true);
      }
    }
    mylcd->turnOn();
  }
}
