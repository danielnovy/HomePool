#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <ESP8266WebServer.h>
#include <uri/UriBraces.h>
#include <LittleFS.h>

#include "Config.h"
#include "GenericEngine.h"
#include "HotEngine.h"

class WebServer {
  
  public:
    WebServer(Config*, GenericEngine*, GenericEngine*, HotEngine*);
    void begin();
    void loop();

  private:
    Config *myConfig;
    GenericEngine *poolEngine;
    GenericEngine *bordaEngine;
    HotEngine *hotEngine;
    String buildResultPage();
    void sendResult();
};

#endif
