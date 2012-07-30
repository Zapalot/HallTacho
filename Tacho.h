#include "Arduino.h"
class Tacho{
public:
  volatile long lastSensorMillis;
  volatile long lastPeriodTime;
  long curVel;  // 1000 means one click per second
  long smoothedVel;
  long smoothFraction;       ///< exponential smoothing. Fraction fraction of the last velocity retained in the new one. 1024 is one...
  long maxVelStepPer16Ms;    ///< the smoothed velocity can only change this much in 10ms
  long lastUpdateMillis;     ///< used to make velocity smoothing a bit less speed dependent
  long minPeriod;            ///< minimum time between two tacho ticks
  
  Tacho(){
    minPeriod=50;
    lastSensorMillis=0;
    lastPeriodTime=0;
    smoothFraction=995;
    lastUpdateMillis=0;
    maxVelStepPer16Ms=10;
    smoothedVel=0.0;
  }

  void update(){
    long curMillis=millis();
    //if the last complete period was shorter than the time since the last sensor-encounter, update interval time to catch a non-spinning
    //wheel.
    if((curMillis-lastSensorMillis)>lastPeriodTime){
      lastPeriodTime=(curMillis-lastSensorMillis);
    }
    
    //calculate velocity based on last sensor period time
    if(lastPeriodTime!=0){
      curVel=1000000.0/lastPeriodTime;
    }
    else{
      curVel=0;
    };
    
    long millisSinceLastUpdate=curMillis- lastUpdateMillis;
    if(millisSinceLastUpdate>16||millisSinceLastUpdate<0){
      smoothedVel=constrain(curVel,smoothedVel-maxVelStepPer16Ms,smoothedVel+maxVelStepPer16Ms);
      lastUpdateMillis=curMillis;
    }
    
    /*
    smoothedVel*=smoothFraction;
    smoothedVel+=(curVel * (1024 - smoothFraction));
    smoothedVel/=1024;
    */
  };
  
  void sensorTurnedOn(){
    long curMillis=millis();
    //a kind of debounce
    if((curMillis-lastSensorMillis)>minPeriod){
      lastPeriodTime=curMillis-lastSensorMillis;
      lastSensorMillis=curMillis;
    };
  };
  
};



