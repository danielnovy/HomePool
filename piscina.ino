#include <EEPROM.h>
#include <LiquidCrystal_I2C.h>
#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>
#include <ESPDateTime.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>

// Conexoes:
// Modulo 4 reles: d7, d6, d5 e tx
// LCD: d2, d1
// Button: rx, gnd
// Thermistor: a0 e gnd
// Resistor 10k: 3.3v -> a0

LiquidCrystal_I2C lcd(0x3F,16,2);  // set the LCD address to 0x3F for a 16 chars and 2 line display


#define HOT_ENGINE_PIN   D6
#define POOL_ENGINE_PIN  D7
#define BORDA_ENGINE_PIN D5
#define SENSOR_SWITCH    1 // TX
#define BUTTON_PIN       3 // RX

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

struct Config {
  byte hotEngineTempDiff;
  byte hotEngineSecondsToRun;

  byte poolEngineStartHour;
  byte poolEngineStartMinute;
  byte poolEngineMinutesToRun;
} myConfig;

bool hotEngineRunning   = false;
bool poolEngineRunning  = false;
bool bordaEngineRunning = false;

bool runBordaWithPool = false;

long hotEngineStartTime;
long poolEngineStartTime;

long lastHotEngineLoop  = 0;
long lastHotEngineStart = 0;
long lastPoolEngineLoop = 0;

long lastLcdBacklight = DateTime.now();
int BACKLIGHT_TIMER = 20;
bool isBacklightOn = true;

float roofTemp;
float poolTemp;
float avgRead;
int areadPool;
int areadRoof;
int httpCode = 0;

bool infoChanged = true;
bool shouldReadPoolTemp = true; // if true, pool temp is read. Otherwise, roof temp is read.

const char* ssid = "Deco";
const char* password = "mobile2008";
WiFiServer server(80);

bool loadConfig() {
  EEPROM.begin(512);
  int addr = 0;
  myConfig.hotEngineTempDiff = EEPROM.read(addr++);
  myConfig.hotEngineSecondsToRun = EEPROM.read(addr++);
  myConfig.poolEngineStartHour = EEPROM.read(addr++);
  myConfig.poolEngineStartMinute = EEPROM.read(addr++);
  myConfig.poolEngineMinutesToRun = EEPROM.read(addr++);
  return true;
}

void saveConfig() {
  int addr = 0;
  EEPROM.write(addr++, myConfig.hotEngineTempDiff);
  EEPROM.write(addr++, myConfig.hotEngineSecondsToRun);
  EEPROM.write(addr++, myConfig.poolEngineStartHour);
  EEPROM.write(addr++, myConfig.poolEngineStartMinute);
  EEPROM.write(addr++, myConfig.poolEngineMinutesToRun);
  EEPROM.commit();
}

void saveTestConfig() {
  
  EEPROM.begin(512);

  myConfig.hotEngineTempDiff = 10;
  myConfig.hotEngineSecondsToRun = 10;
  myConfig.poolEngineStartHour = 7;
  myConfig.poolEngineStartMinute = 0;
  myConfig.poolEngineMinutesToRun = 1;

  saveConfig();

  myConfig.hotEngineTempDiff = 0;
  myConfig.hotEngineSecondsToRun = 0;
  myConfig.poolEngineStartHour = 0;
  myConfig.poolEngineStartMinute = 0;
  myConfig.poolEngineMinutesToRun = 0;
}

void printConfig() {
  p("myConfig.hotEngineTempDiff:      ", myConfig.hotEngineTempDiff);
  p("myConfig.hotEngineSecondsToRun:  ", myConfig.hotEngineSecondsToRun);

  p("myConfig.poolEngineStartHour:    ", myConfig.poolEngineStartHour);
  p("myConfig.poolEngineStartMinute:  ", myConfig.poolEngineStartMinute);
  p("myConfig.poolEngineMinutesToRun: ", myConfig.poolEngineMinutesToRun);
}

bool setupWiFi() {
    // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
 
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
 
  // Start the server
  server.begin();
  Serial.println("Server started");
 
  // Print the IP address
  Serial.print("Use this URL : ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");
  return true;
}

bool setupDevices() {

  lcd.init();
  lcd.backlight();

  pinMode(HOT_ENGINE_PIN,   OUTPUT); // rele 1 - Aquecedor
  pinMode(POOL_ENGINE_PIN,  OUTPUT); // rele 2 - Bomba do Filtro
  pinMode(BORDA_ENGINE_PIN, OUTPUT); // rele 3 - Borda Infinita
  pinMode(SENSOR_SWITCH,    OUTPUT); // rele 4 - Switch do sensor de temperatura

  pinMode(A0, INPUT); // Sensor de temperatura

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  
  // Desliga todos os reles (sim, high eh para desligar :)
  digitalWrite(HOT_ENGINE_PIN,   HIGH);
  digitalWrite(POOL_ENGINE_PIN,  HIGH);
  digitalWrite(BORDA_ENGINE_PIN, HIGH);
  digitalWrite(SENSOR_SWITCH,    LOW); // comeca lendo temp da piscina

  return true;
}

void setupOTA() {
  ArduinoOTA.onStart([]() {
    Serial.println("Inicio...");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("nFim!");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progresso: %u%%r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Erro [%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Autenticacao Falhou");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Falha no Inicio");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Falha na Conexao");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Falha na Recepcao");
    else if (error == OTA_END_ERROR) Serial.println("Falha no Fim");
  });
  ArduinoOTA.begin();
  Serial.println("Pronto");
  Serial.print("Endereco IP: ");
  Serial.println(WiFi.localIP());
}

void setup() {
  
  //saveTestConfig();
  loadConfig();
  printConfig();
  setupDevices();
  setupWiFi();
  setupOTA();

  DateTime.setTimeZone("<-03>3");
  DateTime.setServer("time.windows.com");
  DateTime.begin();


}

void loop() {
  ArduinoOTA.handle();
  webServerLoop();
  hotEngineLoop();
  poolEngineLoop();
  checkLcd();
  checkButtonPressed();
}

long now() {
  return DateTime.now();
}

void checkLcd() {
  if (infoChanged) {
    printInfoLcd();
    infoChanged = false;
  }
  long inow = now();
  if ((inow - lastLcdBacklight) > BACKLIGHT_TIMER) {
    lcd.noBacklight();
    isBacklightOn = false;
  }
}

void checkButtonPressed() {
  if (pressed(BUTTON_PIN)) {
    if (isBacklightOn) {
      if (poolEngineRunning) {
        stopPoolEngine();
      } else {
        startPoolEngine();
      }
    }
    lcd.backlight();
    lastLcdBacklight = now();
    isBacklightOn = true;
  }
}

void printInfoLcd() {
  lcd.setCursor(0, 0);
  lcd.print("     ");
  lcd.setCursor(0, 0);
  lcd.print(roofTemp);
  lcd.setCursor(4, 0);
  lcd.print(" ");
  lcd.setCursor(4, 0);
  lcd.print(" ");
  lcd.setCursor(5, 0);
  lcd.print((hotEngineRunning)   ? "On " : "Off");
  lcd.setCursor(9, 0);
  lcd.print((poolEngineRunning)  ? "On " : "Off");
  lcd.setCursor(13, 0);
  lcd.print((bordaEngineRunning) ? "On " : "Off");

  lcd.setCursor(0, 1);
  lcd.print("     ");
  lcd.setCursor(0, 1);
  lcd.print(poolTemp);
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
              roofTemp,
              poolTemp,
              myConfig.hotEngineTempDiff,
              myConfig.hotEngineSecondsToRun,
              myConfig.poolEngineStartHour,
              myConfig.poolEngineStartMinute,
              myConfig.poolEngineMinutesToRun,
              poolEngineRunning,
              bordaEngineRunning,
              hotEngineRunning
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
  
  long inow = now();
  if (inow - lastHotEngineLoop < 5) {
    // only run once each 5s
    return;
  }
  lastHotEngineLoop = inow;
  
  if (shouldReadPoolTemp) {
    poolTemp = readPoolTemp();
  } else {
    roofTemp = readRoofTemp();
  }
  shouldReadPoolTemp = !shouldReadPoolTemp;
  
  if (!hotEngineRunning) {
#ifdef MODE_ONLY_ROOF_TEMP
    // here, myConfig.hotEngineTempDiff = min roof temp to activate engine
    if (roofTemp >= myConfig.hotEngineTempDiff) {
      startHotEngine();
    }
#else
    if ((roofTemp - poolTemp) > myConfig.hotEngineTempDiff) {
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
  String url = "http://167.172.14.135//cgi-bin/bli.py?who=" + who + "&running=" + String(hotEngineRunning) + "&date=" + date + "&hour=" + hour + "&minute=" + minute + "&temp=" + String(result);
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
  areadPool = getAverageRead();
  float result = convertTemp(areadPool);
  infoChanged = true;
  //postTemp("pool", result);
  digitalWrite(SENSOR_SWITCH, LOW); // Chaveia para sensor do telhado (T1)
  return result;
}

float readRoofTemp() {
  areadRoof = getAverageRead();
  float result = convertTemp(areadRoof);
  infoChanged = true;
  //postTemp("roof", result);
  digitalWrite(SENSOR_SWITCH, HIGH); // chaveia para sensor da piscina (T2)
  return result;
}

void startHotEngine() {
  hotEngineStartTime = now();
  //Serial.println("Starting hot engine...");
  digitalWrite(HOT_ENGINE_PIN, LOW);
  hotEngineRunning = true;
  infoChanged = true;
  lastHotEngineStart = hotEngineStartTime;
}

void stopHotEngine() {
  //Serial.println("Stopping hot engine...");
  digitalWrite(HOT_ENGINE_PIN, HIGH);
  hotEngineRunning = false;
  infoChanged = true;
}

bool checkStopHotEngine() {
  long inow = now();
  long diff = inow - hotEngineStartTime;
  if (diff > myConfig.hotEngineSecondsToRun) {
    return true;
  }
  return false;
}

/////////////////////////////////////
///// POOL ENGINE FUNCTIONS /////////
/////////////////////////////////////

void poolEngineLoop() {
  long inow = now();
  if (inow - lastPoolEngineLoop < 5) {
    // only run once each 5s
    return;
  }
  lastPoolEngineLoop = inow;
  if (!poolEngineRunning) {
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
  poolEngineStartTime = now();
  //Serial.println("Starting pool engine...");
  digitalWrite(POOL_ENGINE_PIN, LOW);
  poolEngineRunning = true;
  infoChanged = true;
}

void stopPoolEngine() {
  //Serial.println("Stopping pool engine...");
  digitalWrite(POOL_ENGINE_PIN, HIGH);
  poolEngineRunning = false;
  infoChanged = true;
}

bool checkStartPoolEngine() {
  bool matchHour = DateTime.format("%H").toInt() == myConfig.poolEngineStartHour;
  bool matchMin  = DateTime.format("%M").toInt() == myConfig.poolEngineStartMinute;
  return matchHour && matchMin;
}

bool checkStopPoolEngine() {
  long inow = now();
  long diff = inow - poolEngineStartTime;
  if (diff > (myConfig.poolEngineMinutesToRun * 60L)) { 
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
  bordaEngineRunning = true;
  infoChanged = true;
}

void stopBordaEngine() {
  //Serial.println("Stopping borda engine...");
  digitalWrite(BORDA_ENGINE_PIN, HIGH);
  bordaEngineRunning = false;
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
      
  result += "<p>Telhado: " + String(roofTemperature) + " (" + String(areadRoof) + ")<br>Piscina: " + String(poolTemperature) + " (" + String(areadPool) + ")</p>";
  
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
    myConfig.hotEngineTempDiff = value;
    saveConfig();
  } else 
  if (command.startsWith("GET /hotEngineSecondsToRun")) {
    int index = command.indexOf("?value=");
    index += 7;
    int value = command.substring(index).toInt();
    myConfig.hotEngineSecondsToRun = value;
    saveConfig();
  } else 
  if (command.startsWith("GET /poolEngineStartHour")) {
    int index = command.indexOf("?value=");
    index += 7;
    int value = command.substring(index).toInt();
    myConfig.poolEngineStartHour = value;
    saveConfig();
  } else 
  if (command.startsWith("GET /poolEngineStartMinute")) {
    int index = command.indexOf("?value=");
    index += 7;
    int value = command.substring(index).toInt();
    myConfig.poolEngineStartMinute = value;
    saveConfig();
  } else 
  if (command.startsWith("GET /poolEngineMinutesToRun")) {
    int index = command.indexOf("?value=");
    index += 7;
    int value = command.substring(index).toInt();
    myConfig.poolEngineMinutesToRun = value;
    saveConfig();
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
    if (poolEngineRunning) {
      startBordaEngine();
    }
  } else 
  if (command.startsWith("GET /bordaWithPoolOff")) {
    runBordaWithPool = false;
    if (poolEngineRunning) {
      stopBordaEngine();
    }
  }
}

void p(String s, long l) {
  Serial.print(s);
  Serial.println(l);
}
