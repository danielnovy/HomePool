#ifndef STATUS_H
#define STATUS_H

class Status {

  private:
    bool poolEngineRunning;
    bool bordaEngineRunning;
    bool hotEngineRunning;
    bool poolLightOn;
    bool poolAlgoOn;
    float roofTemperature;
    float poolTemperature;

  public:
    bool infoChanged;
    void setPoolEngineRunning(bool);
    bool isPoolEngineRunning();
    void setBordaEngineRunning(bool);
    bool isBordaEngineRunning();
    void setHotEngineRunning(bool);
    bool isHotEngineRunning();
    void  setRoofTemperature(float);
    float getRoofTemperature();
    void  setPoolTemperature(float);
    float getPoolTemperature();
    void setPoolLightOn(bool);
    bool isPoolLightOn();
    void setPoolAlgoOn(bool);
    bool isPoolAlgoOn();
    
};

#endif
