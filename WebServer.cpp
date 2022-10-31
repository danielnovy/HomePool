#include "WebServer.h"

ESP8266WebServer server(80);

WebServer::WebServer(Config* myConfig, GenericEngine* poolEngine, GenericEngine* bordaEngine, HotEngine* hotEngine) {
  this->myConfig = myConfig;
  this->poolEngine = poolEngine;
  this->bordaEngine = bordaEngine;
  this->hotEngine = hotEngine;
}

void WebServer::sendResult() {
  server.send(200, "text/html", this->buildResultPage());
}

void WebServer::begin() {

  LittleFS.begin();
  
  server.on("/", [&]() {
    this->sendResult();
  });
  server.on("/poolEngine",  [&]() {
    if (this->poolEngine->running) {
      this->poolEngine->stop();
    } else {
      this->poolEngine->start();
    }
    this->sendResult();
  });
  server.on("/bordaEngine",  [&]() {
    if (this->bordaEngine->running) {
      this->bordaEngine->stop();
    } else {
      this->bordaEngine->start();
    }
    this->sendResult();
  });
  server.on("/hotEngine",  [&]() {
    if (this->hotEngine->running) {
      this->hotEngine->stop();
    } else {
      this->hotEngine->start();
    }
    this->sendResult();
  });
  server.on("/saveConfig", [&]() {
    this->myConfig->hotEngineTempDiff       = server.arg("hotEngineTempDiff").toInt();
    this->myConfig->hotEngineSecondsToRun   = server.arg("hotEngineSecondsToRun").toInt();
    this->myConfig->poolEngineStartHour     = server.arg("poolEngineStartHour").toInt();
    this->myConfig->poolEngineStartMinute   = server.arg("poolEngineStartMinute").toInt();
    this->myConfig->poolEngineMinutesToRun  = server.arg("poolEngineMinutesToRun").toInt();
    this->myConfig->bordaEngineMinutesToRun = server.arg("bordaEngineMinutesToRun").toInt();
    this->myConfig->save();
    this->sendResult();
  });
  server.begin();
}

void WebServer::loop() {
  server.handleClient();
}

String WebServer::buildResultPage() {
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
  
  result.replace("{{POOL_ENGINE_START_HOUR}}", String(this->myConfig->poolEngineStartHour));
  result.replace("{{POOL_ENGINE_START_MINUTE}}", String(this->myConfig->poolEngineStartMinute));
  result.replace("{{POOL_ENGINE_MINUTES_TO_RUN}}", String(this->myConfig->poolEngineMinutesToRun));
  result.replace("{{BORDA_ENGINE_MINUTES_TO_RUN}}", String(this->myConfig->bordaEngineMinutesToRun));
  result.replace("{{HOT_ENGINE_TEMP_DIFF}}", String(this->myConfig->hotEngineTempDiff));
  result.replace("{{HOT_ENGINE_SECONDS_TO_RUN}}", String(this->myConfig->hotEngineSecondsToRun));
  
  return result;
}
