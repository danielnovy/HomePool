#include "WebServer.h"

ESP8266WebServer server(80);

WebServer::WebServer(Status *status, Config* myConfig, GenericEngine* poolEngine, GenericEngine* bordaEngine, HotEngine* hotEngine, PoolLight *poolLight) {
  this->status = status;
  this->myConfig = myConfig;
  this->poolEngine = poolEngine;
  this->bordaEngine = bordaEngine;
  this->hotEngine = hotEngine;
  this->poolLight = poolLight;
}

void WebServer::sendResult() {
  server.send(200, "text/html", this->buildResultPage());
}

void WebServer::begin() {
  
  this->lastBoot = DateTime.toString();

  Serial.println("Before LittleFS.begin() ");
  LittleFS.begin();
  Serial.println("After LittleFS.begin() ");
  
  server.on("/", [&]() {
    this->sendResult();
  });
  server.on("/poolEngine",  [&]() {
    if (this->poolEngine->running) {
      this->poolEngine->stop();
      this->status->setPoolEngineRunning(false);
    } else {
      this->poolEngine->start();
      this->status->setPoolEngineRunning(true);
    }
    this->sendResult();
  });
  server.on("/bordaEngine",  [&]() {
    if (this->bordaEngine->running) {
      this->bordaEngine->stop();
      this->status->setBordaEngineRunning(false);
    } else {
      this->bordaEngine->start();
      this->status->setBordaEngineRunning(true);
    }
    this->sendResult();
  });
  server.on("/hotEngine",  [&]() {
    if (this->hotEngine->running) {
      this->hotEngine->stop();
      this->status->setHotEngineRunning(false);
    } else {
      this->hotEngine->start();
      this->status->setHotEngineRunning(true);
    }
    this->sendResult();
  });
  server.on("/poolLight",  [&]() {
    if (this->status->isPoolLightOn()) {
      this->poolLight->turnOff();
      this->status->setPoolLightOn(false);
    } else {
      this->poolLight->turnOn();
      this->status->setPoolLightOn(true);
    }
    this->sendResult();
  });
  server.on("/poolAlgo",  [&]() {
    if (this->status->isPoolAlgoOn()) {
      this->poolLight->turnAlgoOff();
    } else {
      this->poolLight->turnAlgoOn();
      this->status->setPoolLightOn(false);
    }
    this->sendResult();
  });
  server.on("/saveConfig", [&]() {
    this->myConfig->hotEngineTempDiff       = server.arg("hotEngineTempDiff").toInt();
    this->myConfig->hotEngineSecondsToRun   = server.arg("hotEngineSecondsToRun").toInt();
    this->myConfig->poolEngineStartHour     = server.arg("poolEngineStartHour").toInt();
    this->myConfig->poolEngineStartMinute   = server.arg("poolEngineStartMinute").toInt();
    this->myConfig->poolEngineHoursToRun    = server.arg("poolEngineHoursToRun").toInt();
    this->myConfig->bordaEngineMinutesToRun = server.arg("bordaEngineMinutesToRun").toInt();
    this->myConfig->poolLightStartHour      = server.arg("poolLightStartHour").toInt();
    this->myConfig->poolLightStartMinute    = server.arg("poolLightStartMinute").toInt();
    this->myConfig->poolLightMinutesToRun   = server.arg("poolLightMinutesToRun").toInt();
    this->myConfig->poolRed   = server.arg("poolRed").toInt();
    this->myConfig->poolGreen = server.arg("poolGreen").toInt();
    this->myConfig->poolBlue  = server.arg("poolBlue").toInt();
    this->myConfig->save();
    if (this->status->isPoolLightOn()) {
      this->poolLight->turnOn(); // update colors
    }
    this->sendResult();
  });
  server.begin();
}

void WebServer::loop() {
  server.handleClient();
}

String WebServer::buildResultPage() {
  if (!LittleFS.exists("/index.html")) {
    Serial.println("Ops! File does not exist!");
  } else {
    Serial.println("FOUND FILE!!!");

  }
  File file = LittleFS.open("/index.html", "r");
  String result = "";
  while (file.available()) {
    result += char(file.read());
  }

  if (this->poolEngine->running) {
    result.replace("{{POOL_ENGINE}}", "checked");
  }

  if (this->bordaEngine->running) {
    result.replace("{{BORDA_ENGINE}}", "checked");
  }
  
  if (this->hotEngine->running) {
    result.replace("{{HOT_ENGINE}}", "checked");
  }

  if (this->status->isPoolLightOn()) {
    result.replace("{{POOL_LIGHT}}", "checked");
  }

  if (this->status->isPoolAlgoOn()) {
    result.replace("{{POOL_ALGO}}", "checked");
  }
  
  result.replace("{{POOL_ENGINE_START_HOUR}}",      String(this->myConfig->poolEngineStartHour));
  result.replace("{{POOL_ENGINE_START_MINUTE}}",    String(this->myConfig->poolEngineStartMinute));
  result.replace("{{POOL_ENGINE_HOURS_TO_RUN}}",    String(this->myConfig->poolEngineHoursToRun));
  result.replace("{{BORDA_ENGINE_MINUTES_TO_RUN}}", String(this->myConfig->bordaEngineMinutesToRun));
  result.replace("{{HOT_ENGINE_TEMP_DIFF}}",        String(this->myConfig->hotEngineTempDiff));
  result.replace("{{POOL_LIGHT_START_HOUR}}",       String(this->myConfig->poolLightStartHour));
  result.replace("{{POOL_LIGHT_START_MINUTE}}",     String(this->myConfig->poolLightStartMinute));
  result.replace("{{POOL_LIGHT_MINUTES_TO_RUN}}",   String(this->myConfig->poolLightMinutesToRun));
  result.replace("{{POOL_RED}}",                    String(this->myConfig->poolRed));
  result.replace("{{POOL_GREEN}}",                  String(this->myConfig->poolGreen));
  result.replace("{{POOL_BLUE}}",                   String(this->myConfig->poolBlue));
  result.replace("{{HOT_ENGINE_SECONDS_TO_RUN}}",   String(this->myConfig->hotEngineSecondsToRun));
  result.replace("{{POOL_TEMPERATURE}}",            String(this->hotEngine->poolTemperature));
  result.replace("{{ROOF_TEMPERATURE}}",            String(this->hotEngine->roofTemperature));
  
  result.replace("{{LAST_BOOT}}", this->lastBoot);

  return result;
}
