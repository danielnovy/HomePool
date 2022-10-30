#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#include "Config.h"
#include "Setup.h"
#include "Status.h"
#include "MyLCD.h"

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
MyLCD  *mylcd    = new MyLCD();
struct Status currStatus;

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

  mylcd = new MyLCD();

  server.begin();
}

void loop() {
  ArduinoOTA.handle();
  webServerLoop();
  hotEngineLoop();
  poolEngineLoop();
  mylcd->loop(infoChanged, currStatus);
  infoChanged = false;
  checkButtonPressed();
}

void checkButtonPressed() {
  if (pressed(BUTTON_PIN)) {
    if (mylcd->isBacklightOn) {
      if (currStatus.poolEngineRunning) {
        stopPoolEngine();
      } else {
        startPoolEngine();
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
            webClient.print(buildResponsePage(
              currStatus.roofTemp,
              currStatus.poolTemp,
              myConfig->hotEngineTempDiff,
              myConfig->hotEngineSecondsToRun,
              myConfig->poolEngineStartHour,
              myConfig->poolEngineStartMinute,
              myConfig->poolEngineMinutesToRun,
              currStatus.poolEngineRunning,
              currStatus.bordaEngineRunning,
              currStatus.hotEngineRunning
            ));
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
///// HOT ENGINE FUNCTIONS //////////
/////////////////////////////////////
void hotEngineLoop() {
  
  long inow = DateTime.now();
  if (inow - lastHotEngineLoop < 5) {
    // only run once each 5s
    return;
  }
  lastHotEngineLoop = inow;
  
  if (shouldReadPoolTemp) {
    currStatus.poolTemp = readPoolTemp();
  } else {
    currStatus.roofTemp = readRoofTemp();
  }
  shouldReadPoolTemp = !shouldReadPoolTemp;
  
  if (!currStatus.hotEngineRunning) {
#ifdef MODE_ONLY_ROOF_TEMP
    // here, myConfig->hotEngineTempDiff = min roof temp to activate engine
    if (currStatus.roofTemp >= myConfig->hotEngineTempDiff) {
      startHotEngine();
    }
#else
    if ((currStatus.roofTemp - currStatus.poolTemp) > myConfig->hotEngineTempDiff) {
      startHotEngine();
    }
#endif
  } else {
    if (checkStopHotEngine()) {
      stopHotEngine();
    }
  }
}

const int reads[]    = {275, 403,  504,  511,  527,  539,  555,  564,  577,  600,  610,  629,  651,  686,  703,  716,  756,  779,  804,  839,  857,  872,  888,  905};
const double temps[] = {  3,  14, 22.3,   23, 24.4, 25.1, 26.5, 27.5, 28.5, 30.5, 31.5, 32.8, 34.5, 38.5, 40.0, 41.1, 44.9, 47.6, 50.2, 55.0, 57.6, 60.0, 62.5, 64.5}; 

float convertTemp(int aread) {
  int slot = -1;
  int reads_size = sizeof(reads);
  for (int i = 1; i < reads_size - 1; i++) {
    if (aread >= reads[i-1] && aread < reads[i]) {
      slot = i - 1;
      break;
    }
  }
  if (slot == -1) return -1;
  double tempdiff = temps[slot+1] - temps[slot];
  int readdiff = reads[slot+1] - reads[slot];
  double increment = tempdiff / readdiff;
  return (increment * (aread - reads[slot]) + temps[slot]);
}

void postTemp(String who, float result) {
  String date = DateTime.format("%Y%m%d");
  String hour = DateTime.format("%H");
  String minute = DateTime.format("%M");
  String url = "http://167.172.14.135//cgi-bin/bli.py?who=" + who + "&running=" + String(currStatus.hotEngineRunning) + "&date=" + date + "&hour=" + hour + "&minute=" + minute + "&temp=" + String(result);
  WiFiClient client;
  HTTPClient http;
  http.begin(client, url);
  httpCode = http.GET();
}

int getAverageRead() {
  int times = 10;
  int result = 0;
  for (int i = 0; i < times; i++) {
    result += analogRead(A0);
    delay(10);
  }
  return (result / times);
}

float readPoolTemp() {
  currStatus.areadPool = getAverageRead();
  float result = convertTemp(currStatus.areadPool);
  infoChanged = true;
  //postTemp("pool", result);
  digitalWrite(SENSOR_SWITCH, LOW); // Chaveia para sensor do telhado (T1)
  return result;
}

float readRoofTemp() {
  currStatus.areadRoof = getAverageRead();
  float result = convertTemp(currStatus.areadRoof);
  infoChanged = true;
  //postTemp("roof", result);
  digitalWrite(SENSOR_SWITCH, HIGH); // chaveia para sensor da piscina (T2)
  return result;
}

void startHotEngine() {
  hotEngineStartTime = DateTime.now();
  //Serial.println("Starting hot engine...");
  digitalWrite(HOT_ENGINE_PIN, LOW);
  currStatus.hotEngineRunning = true;
  infoChanged = true;
  lastHotEngineStart = hotEngineStartTime;
}

void stopHotEngine() {
  //Serial.println("Stopping hot engine...");
  digitalWrite(HOT_ENGINE_PIN, HIGH);
  currStatus.hotEngineRunning = false;
  infoChanged = true;
}

bool checkStopHotEngine() {
  long inow = DateTime.now();
  long diff = inow - hotEngineStartTime;
  if (diff > myConfig->hotEngineSecondsToRun) {
    return true;
  }
  return false;
}

/////////////////////////////////////
///// POOL ENGINE FUNCTIONS /////////
/////////////////////////////////////

void poolEngineLoop() {
  long inow = DateTime.now();
  if (inow - lastPoolEngineLoop < 5) {
    // only run once each 5s
    return;
  }
  lastPoolEngineLoop = inow;
  if (!currStatus.poolEngineRunning) {
    if (checkStartPoolEngine()) {
      startPoolEngine();
      if (runBordaWithPool) {
        startBordaEngine();
      }
    }
  } else {
    if (checkStopPoolEngine()) {
      stopPoolEngine();
      if (runBordaWithPool) {
        stopBordaEngine();
      }
    }
  }
}

void startPoolEngine() {
  poolEngineStartTime = DateTime.now();
  //Serial.println("Starting pool engine...");
  digitalWrite(POOL_ENGINE_PIN, LOW);
  currStatus.poolEngineRunning = true;
  infoChanged = true;
}

void stopPoolEngine() {
  //Serial.println("Stopping pool engine...");
  digitalWrite(POOL_ENGINE_PIN, HIGH);
  currStatus.poolEngineRunning = false;
  infoChanged = true;
}

bool checkStartPoolEngine() {
  bool matchHour = DateTime.format("%H").toInt() == myConfig->poolEngineStartHour;
  bool matchMin  = DateTime.format("%M").toInt() == myConfig->poolEngineStartMinute;
  return matchHour && matchMin;
}

bool checkStopPoolEngine() {
  long inow = DateTime.now();
  long diff = inow - poolEngineStartTime;
  if (diff > (myConfig->poolEngineMinutesToRun * 60L)) { 
    return true;
  }
  return false;
}

/////////////////////////////////////
///// BORDA ENGINE FUNCTIONS ////////
/////////////////////////////////////

void startBordaEngine() {
  //Serial.println("Starting borda engine...");
  digitalWrite(BORDA_ENGINE_PIN, LOW);
  currStatus.bordaEngineRunning = true;
  infoChanged = true;
}

void stopBordaEngine() {
  //Serial.println("Stopping borda engine...");
  digitalWrite(BORDA_ENGINE_PIN, HIGH);
  currStatus.bordaEngineRunning = false;
  infoChanged = true;
}

/////////////////////////////////////
///// UTILS /////////////////////////
/////////////////////////////////////

long lastPressed = 0;
bool pressed(int pin) {
  long inow = millis();
  if (inow - lastPressed > 500L) {
    if (digitalRead(pin) == LOW) {
      lastPressed = inow;
      return true;
    }
  }
  return false;
}

String buildResponsePage(
    float roofTemperature,
    float poolTemperature,
    byte hotEngineTempDiff,
    byte hotEngineSecondsToRun,
    byte poolEngineStartHour,
    byte poolEngineStartMinute,
    byte poolEngineMinutesToRun,
    bool isPoolEngineOn,
    bool isBordaEngineOn,
    bool isHotEngineOn
  ) {
  String result = "HTTP/1.1 200 OK\n";
  result += "Content-Type: text/html\n";
  result += "Connection: close\n\n";
  result += "<!DOCTYPE HTML>\n";
  result += "<html><header><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"></header>";
  result += "<body><h1>Controlador Piscina</h1>";
  
  result += "<p>" + DateTime.format("%F %T") + "</p>";
      
  result += "<p>Telhado: " + String(roofTemperature) + " (" + String(currStatus.areadRoof) + ")<br>Piscina: " + String(poolTemperature) + " (" + String(currStatus.areadPool) + ")</p>";
  
  result += "<h2><a href=\"/\">REFRESH</a></h2>";
  result += "<h3>Configuracao da bomba de aquecimento</h3>";
#ifdef MODE_ONLY_ROOF_TEMP
  result += "<h4>Temperatura do telhado para ligar bomba: <form action=\"/hotEngineTempDiff\">";
#else 
  result += "<h4>Diff de temp para ligar bomba: <form action=\"/hotEngineTempDiff\">";
#endif
  result += "<input name=\"value\" value=\"" + String(hotEngineTempDiff) +"\"><input type=submit></form></h4>";
  result += "<h4>Tempo (em seg) ligada: <form action=\"/hotEngineSecondsToRun\">";
  result += "<input name=\"value\" value=\"" + String(hotEngineSecondsToRun) +"\"><input type=submit></form></h4>";
  result += "<h3>Configuracao da bomba de filtragem</h3>";
  result += "<h4>Hora para ligar: <form action=\"/poolEngineStartHour\">";
  result += "<input name=\"value\" value=\"" + String(poolEngineStartHour) +"\"><input type=submit></form></h4>";
  result += "<h4>Minuto para ligar: <form action=\"/poolEngineStartMinute\">";
  result += "<input name=\"value\" value=\"" + String(poolEngineStartMinute) +"\"><input type=submit></form></h4>";
  result += "<h4>Tempo (em minutos) para executar: <form action=\"/poolEngineMinutesToRun\">";
  result += "<input name=\"value\" value=\"" + String(poolEngineMinutesToRun) +"\"><input type=submit></form></h4>";

  result += "<h2>Ligar/Desligar bombas</h2>";
  if (isPoolEngineOn) {
    result += "<a href=\"/poolEngineOff\">Desligar bomba de filtragem</a>";
  } else {
    result += "<a href=\"/poolEngineOn\">Ligar bomba de filtragem</a>";
  }
  result += "<br><br>";
  if (isBordaEngineOn) {
    result += "<a href=\"/bordaEngineOff\">Desligar bomba da borda infinita</a>";
  } else {
    result += "<a href=\"/bordaEngineOn\">Ligar bomba da borda infinita</a>";
  }
  result += "<br><br>";
  if (isHotEngineOn) {
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
  
  if (command.startsWith("GET /poolEngineOff")) {
    stopPoolEngine();
  } else 
  if (command.startsWith("GET /poolEngineOn")) {
    startPoolEngine();
  } else
  if (command.startsWith("GET /bordaEngineOff")) {
    stopBordaEngine();
  } else
  if (command.startsWith("GET /bordaEngineOn")) {
    startBordaEngine();
  }
  if (command.startsWith("GET /hotEngineOff")) {
    stopHotEngine();
  } else
  if (command.startsWith("GET /hotEngineOn")) {
    startHotEngine();
  } else
  if (command.startsWith("GET /bordaWithPoolOn")) {
    runBordaWithPool = true;
    if (currStatus.poolEngineRunning) {
      startBordaEngine();
    }
  } else 
  if (command.startsWith("GET /bordaWithPoolOff")) {
    runBordaWithPool = false;
    if (currStatus.poolEngineRunning) {
      stopBordaEngine();
    }
  }
}
