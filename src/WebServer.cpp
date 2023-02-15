#include "WebServer.h"

ESP8266WebServer server(80);

String pageString = "";

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
  if (pageString.length() == 0) {
    File file = LittleFS.open("/index.html", "r");
    while (file.available()) {
      pageString += char(file.read());
    }
  }

  String result = pageString;

  if (this->poolEngine->running) {
    result.replace("{{1}}", "checked");
  }

  if (this->bordaEngine->running) {
    result.replace("{{2}}", "checked");
  }
  
  if (this->hotEngine->running) {
    result.replace("{{3}}", "checked");
  }

  if (this->status->isPoolLightOn()) {
    result.replace("{{4}}", "checked");
  }

  if (this->status->isPoolAlgoOn()) {
    result.replace("{{5}}", "checked");
  }
  
  result.replace("{{6}}",      String(this->myConfig->poolEngineStartHour));
  result.replace("{{7}}",    String(this->myConfig->poolEngineStartMinute));
  result.replace("{{8}}",    String(this->myConfig->poolEngineHoursToRun));
  result.replace("{{9}}", String(this->myConfig->bordaEngineMinutesToRun));
  result.replace("{{10}}",        String(this->myConfig->hotEngineTempDiff));
  result.replace("{{11}}",       String(this->myConfig->poolLightStartHour));
  result.replace("{{12}}",     String(this->myConfig->poolLightStartMinute));
  result.replace("{{13}}",   String(this->myConfig->poolLightMinutesToRun));
  result.replace("{{14}}",                    String(this->myConfig->poolRed));
  result.replace("{{15}}",                  String(this->myConfig->poolGreen));
  result.replace("{{16}}",                   String(this->myConfig->poolBlue));
  result.replace("{{17}}",   String(this->myConfig->hotEngineSecondsToRun));
  result.replace("{{18}}",            String(this->hotEngine->poolTemperature));
  result.replace("{{19}}",            String(this->hotEngine->roofTemperature));
  
  result.replace("{{20}}", this->lastBoot);

  return result;
}
