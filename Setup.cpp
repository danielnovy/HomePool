#include "Setup.h"

const char* ssid = "Deco";
const char* password = "mobile2008";

void setupDevices() {

  pinMode(HOT_ENGINE_PIN,   OUTPUT); // rele 1 - Aquecedor
  pinMode(POOL_ENGINE_PIN,  OUTPUT); // rele 2 - Bomba do Filtro
  pinMode(BORDA_ENGINE_PIN, OUTPUT); // rele 3 - Borda Infinita
  pinMode(SENSOR_SWITCH,    OUTPUT); // rele 4 - Switch do sensor de temperatura

  pinMode(A0, INPUT); // Sensor de temperatura
  
  // Desliga todos os reles (sim, high eh para desligar :)
  digitalWrite(HOT_ENGINE_PIN,   HIGH);
  digitalWrite(POOL_ENGINE_PIN,  HIGH);
  digitalWrite(BORDA_ENGINE_PIN, HIGH);
  digitalWrite(SENSOR_SWITCH,    LOW); // comeca lendo temp da piscina
}

void setupWiFi() {
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
 
  // Print the IP address
  Serial.print("Use this URL : ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");
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

void setupDateTime() {
  DateTime.setTimeZone("<-03>3");
  DateTime.setServer("time.windows.com");
  DateTime.begin();
}

void setupAll() {
  setupWiFi();
  setupOTA();
  setupDateTime();
  setupDevices();
}
