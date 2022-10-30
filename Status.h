#ifndef STATUS_H
#define STATUS_H

struct Status {
  bool hotEngineRunning   = false;
  bool poolEngineRunning  = false;
  bool bordaEngineRunning = false;
  float roofTemp;
  float poolTemp;
  int areadPool;
  int areadRoof;
};

#endif
