#include "Arduino.h"
#include <ArdParameterSettings.h>
extern OSCServer oscServer;

long turnResetTime;      //after so many millis without a turn, the accumulated distance will be reset to zero
long EEMEM persistentTurnResetTime=0;   // the compiler will give this a unique adress in eeprom, but reading and writing must bedone manually
LongParameterSetting turnResetTimeSetting;

long turnDecayRate;      //every 16 Milliseconds, the turn counter will be decreased by this number
long EEMEM persistentTurnDecayRate=0;   // the compiler will give this a unique adress in eeprom, but reading and writing must bedone manually
LongParameterSetting turnDecayRateSetting;

long maxVelStepPer16Ms;      //the smoothed velocity will only change so much in 16 milliseconds
long EEMEM persistentMaxVelStepPer16Ms=0;   // the compiler will give this a unique adress in eeprom, but reading and writing must bedone manually
LongParameterSetting maxVelStepPer16MsSetting;


class Tacho{
public:
  volatile long lastSensorMillis;
  volatile long lastPeriodTime;
  long curVel;  // 1000 means one click per second
  volatile long accumulatedTurns;    ///< turns since start*1000, possibly with a decay
  long smoothedVel;
  long smoothFraction;       ///< exponential smoothing. Fraction fraction of the last velocity retained in the new one. 1024 is one...
  long lastUpdateMillis;     ///< used to make velocity smoothing a bit less speed dependent
  long minPeriod;            ///< minimum time between two tacho ticks
  
  Tacho(){
    minPeriod=50;
    lastSensorMillis=0;
    lastPeriodTime=0;
    smoothFraction=995;
    lastUpdateMillis=0;
    smoothedVel=0.0;
  }
  void setup(){
    turnResetTimeSetting.setup(&turnResetTime,0,100000,F("turns/resetTime"),&oscServer,(int)&persistentTurnResetTime);
    turnDecayRateSetting.setup(&turnDecayRate,0,100000,F("turns/decay"),&oscServer,(int)&persistentTurnDecayRate);
    maxVelStepPer16MsSetting.setup(&maxVelStepPer16Ms,0,1000,F("speed/maxChange"),&oscServer,(int)&persistentMaxVelStepPer16Ms);
  }
  void printState(){
    Serial.print(digitalRead(2));
    Serial.print("\t cur Vel: ");
    Serial.print(curVel);
    Serial.print("\t smoothed Vel: ");
    Serial.print(smoothedVel);
    Serial.print("\t last delta t: ");
    Serial.println(lastPeriodTime);
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
    
    if(lastPeriodTime>turnResetTime)accumulatedTurns=0;
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
      accumulatedTurns+=1000;
    };
  };
  
};



