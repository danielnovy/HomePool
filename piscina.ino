#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#include "Config.h"
#include "Setup.h"
#include "MyLCD.h"
#include "Button.h"
#include "GenericEngine.h"
#include "HotEngine.h"

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

GenericEngine *poolEngine  = new GenericEngine(POOL_ENGINE_PIN, true);
GenericEngine *bordaEngine = new GenericEngine(BORDA_ENGINE_PIN, false);
HotEngine     *hotEngine   = new HotEngine(HOT_ENGINE_PIN, THERMISTOR_SWITCH_PIN);

MyLCD  *mylcd = new MyLCD(poolEngine, bordaEngine, hotEngine);

bool runBordaWithPool = true;

long hotEngineStartTime;
long poolEngineStartTime;

long lastHotEngineLoop  = 0;
long lastHotEngineStart = 0;
long lastPoolEngineLoop = 0;

float avgRead;

int httpCode = 0;

bool infoChanged = true;
bool shouldReadPoolTemp = true; // if true, pool temp is read. Otherwise, roof temp is read.

WiFiServer server(80);

void setup() {
  //myConfig->saveTest();
  myConfig->load();
  setupAll();
  server.begin();
  button->begin();
  poolEngine->begin(myConfig);
  bordaEngine->begin(myConfig);
  hotEngine->begin(myConfig);
}

void loop() {
  ArduinoOTA.handle();
  webServerLoop();
  mylcd->updateScreen(poolEngine->loop());
  mylcd->updateScreen(bordaEngine->loop());
  mylcd->updateScreen(hotEngine->loop());
  checkButtonPressed();
}

void checkButtonPressed() {
  if (button->isPressed()) {
    if (mylcd->isBacklightOn) {
      if (poolEngine->running) {
        poolEngine->stop();
      } else {
        poolEngine->start();
      }
    }
    mylcd->turnOn();
  }
}

void webServerLoop() {
  WiFiClient webClient = server.available();
  if (webClient) {
    String line = "";
    String header = "";
    while (webClient.connected()) {
      if (webClient.available()) {
        char c = webClient.read();
        header += c;
        if (c == '\n') {
          if (line.length() == 0) {
            // end of http request, send response back based on command received
            webCommand(header);
            webClient.print(buildResponsePage());
            break; // break while loop
          } else {
            Serial.println(line);
            line = "";
          }
        } else if (c != '\r') {
          line += c;
        }
      }
    }
    webClient.stop();
  }
}

/////////////////////////////////////
///// UTILS /////////////////////////
/////////////////////////////////////


String buildResponsePage() {
  String result = "HTTP/1.1 200 OK\n";
  result += "Content-Type: text/html\n";
  result += "Connection: close\n\n";
  result += "<!DOCTYPE HTML>\n";
  result += "<html><header><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"></header>";
  result += "<body><h1>Controlador Piscina</h1>";
  
  result += "<p>" + DateTime.format("%F %T") + "</p>";
      
  result += "<p>Telhado: " + String(hotEngine->roofTemperature) + " (" + String(hotEngine->lastRoofRead) + ")<br>Piscina: " + String(hotEngine->poolTemperature) + " (" + String(hotEngine->lastPoolRead) + ")</p>";
  
  result += "<h2><a href=\"/\">REFRESH</a></h2>";
  result += "<h3>Configuracao da bomba de aquecimento</h3>";
#ifdef MODE_ONLY_ROOF_TEMP
  result += "<h4>Temperatura do telhado para ligar bomba: <form action=\"/hotEngineTempDiff\">";
#else 
  result += "<h4>Diff de temp para ligar bomba: <form action=\"/hotEngineTempDiff\">";
#endif
  result += "<input name=\"value\" value=\"" + String(myConfig->hotEngineTempDiff) +"\"><input type=submit></form></h4>";
  result += "<h4>Tempo (em seg) ligada: <form action=\"/hotEngineSecondsToRun\">";
  result += "<input name=\"value\" value=\"" + String(myConfig->hotEngineSecondsToRun) +"\"><input type=submit></form></h4>";
  result += "<h3>Configuracao da bomba de filtragem</h3>";
  result += "<h4>Hora para ligar: <form action=\"/poolEngineStartHour\">";
  result += "<input name=\"value\" value=\"" + String(myConfig->poolEngineStartHour) +"\"><input type=submit></form></h4>";
  result += "<h4>Minuto para ligar: <form action=\"/poolEngineStartMinute\">";
  result += "<input name=\"value\" value=\"" + String(myConfig->poolEngineStartMinute) +"\"><input type=submit></form></h4>";
  result += "<h4>Minutos para executar filtragem: <form action=\"/poolEngineMinutesToRun\">";
  result += "<input name=\"value\" value=\"" + String(myConfig->poolEngineMinutesToRun) +"\"><input type=submit></form></h4>";
  result += "<h4>Minutos para executar borda: <form action=\"/bordaEngineMinutesToRun\">";
  result += "<input name=\"value\" value=\"" + String(myConfig->bordaEngineMinutesToRun) +"\"><input type=submit></form></h4>";

  result += "<h2>Ligar/Desligar bombas</h2>";
  if (poolEngine->running) {
    result += "<a href=\"/poolEngineOff\">Desligar bomba de filtragem</a>";
  } else {
    result += "<a href=\"/poolEngineOn\">Ligar bomba de filtragem</a>";
  }
  result += "<br><br>";
  if (bordaEngine->running) {
    result += "<a href=\"/bordaEngineOff\">Desligar bomba da borda infinita</a>";
  } else {
    result += "<a href=\"/bordaEngineOn\">Ligar bomba da borda infinita</a>";
  }
  result += "<br><br>";
  if (bordaEngine->running) {
    result += "<a href=\"/hotEngineOff\">Desligar bomba de aquecimento</a>";
  } else {
    result += "<a href=\"/hotEngineOn\">Ligar bomba de aquecimento</a>";
  }
  result += "<br><br>";
  if (runBordaWithPool) {
    result += "<a href=\"/bordaWithPoolOff\">Desabilitar borda com filtro</a>";
  } else {
    result += "<a href=\"/bordaWithPoolOn\">Habilitar borda com filtro</a>";
  }
  result += "</body></html>\n\n";

  return result;
}

void webCommand(String command) {

  if (command.startsWith("GET /hotEngineTempDiff")) {
    int index = command.indexOf("?value=");
    index += 7;
    int value = command.substring(index).toInt();
    myConfig->hotEngineTempDiff = value;
    myConfig->save();
  } else 
  if (command.startsWith("GET /hotEngineSecondsToRun")) {
    int index = command.indexOf("?value=");
    index += 7;
    int value = command.substring(index).toInt();
    myConfig->hotEngineSecondsToRun = value;
    myConfig->save();
  } else 
  if (command.startsWith("GET /poolEngineStartHour")) {
    int index = command.indexOf("?value=");
    index += 7;
    int value = command.substring(index).toInt();
    myConfig->poolEngineStartHour = value;
    myConfig->save();
  } else 
  if (command.startsWith("GET /poolEngineStartMinute")) {
    int index = command.indexOf("?value=");
    index += 7;
    int value = command.substring(index).toInt();
    myConfig->poolEngineStartMinute = value;
    myConfig->save();
  } else 
  if (command.startsWith("GET /poolEngineMinutesToRun")) {
    int index = command.indexOf("?value=");
    index += 7;
    int value = command.substring(index).toInt();
    myConfig->poolEngineMinutesToRun = value;
    myConfig->save();
  } else
  if (command.startsWith("GET /bordaEngineMinutesToRun")) {
    int index = command.indexOf("?value=");
    index += 7;
    int value = command.substring(index).toInt();
    myConfig->bordaEngineMinutesToRun = value;
    myConfig->save();
  } else
  
  if (command.startsWith("GET /poolEngineOff")) {
    poolEngine->stop();
  } else 
  if (command.startsWith("GET /poolEngineOn")) {
    poolEngine->start();
  } else
  if (command.startsWith("GET /bordaEngineOff")) {
    bordaEngine->stop();
  } else
  if (command.startsWith("GET /bordaEngineOn")) {
    bordaEngine->start();
  }
  if (command.startsWith("GET /hotEngineOff")) {
    hotEngine->stop();
  } else
  if (command.startsWith("GET /hotEngineOn")) {
    hotEngine->start();
  } else
  if (command.startsWith("GET /bordaWithPoolOn")) {
    runBordaWithPool = true;
    if (poolEngine->running) {
      bordaEngine->start();
    }
  } else 
  if (command.startsWith("GET /bordaWithPoolOff")) {
    runBordaWithPool = false;
    if (poolEngine->running) {
      bordaEngine->stop();
    }
  }
}
