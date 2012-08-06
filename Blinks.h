#include "Arduino.h"
#include <ArdParameterSettings.h>
extern OSCServer oscServer;

#define RL_N_CHANNELS 16
const int RL_CH_INDICES[]= {
  22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37};

//standalone
long standAloneChaseDelay;
long EEMEM persistentStandAloneChaseDelay=0;   // the compiler will give this a unique adress in eeprom, but reading and writing must bedone manually
LongParameterSetting StandAloneChaseDelaySetting;

//tusch
long tuschDuration;
long EEMEM persistentTuschDuration=0;   // the compiler will give this a unique adress in eeprom, but reading and writing must bedone manually
LongParameterSetting TuschDurationSetting;

//single moving light
long singleLightEndSpeed;
long EEMEM persistentSingleLightEndSpeed=0;   // the compiler will give this a unique adress in eeprom, but reading and writing must bedone manually
LongParameterSetting SingleLightEndSpeedSetting;

//fillup
long fillUpFallBackTimeOut;
long EEMEM persistentFillUpFallBackTimeOut=0;   // the compiler will give this a unique adress in eeprom, but reading and writing must bedone manually
LongParameterSetting FillUpFallBackTimeOutSetting;

long fillUpEndDistance;
long EEMEM persistentFillUpEndDistance=0;   // the compiler will give this a unique adress in eeprom, but reading and writing must bedone manually
LongParameterSetting FillUpEndDistanceSetting;

//chaser
long chaserDistDecay;
long EEMEM persistentChaserDistDecay=0;   // the compiler will give this a unique adress in eeprom, but reading and writing must bedone manually
LongParameterSetting ChaserDistDecaySetting;

long chaserMaxSpeed;
long EEMEM persistentChaserMaxSpeed=0;   // the compiler will give this a unique adress in eeprom, but reading and writing must bedone manually
LongParameterSetting ChaserMaxSpeedSetting;

long chaserEndDistance;
long EEMEM persistentChaserEndDistance=0;   // the compiler will give this a unique adress in eeprom, but reading and writing must bedone manually
LongParameterSetting ChaserEndDistanceSetting;

long chaserEndMinSpeed;
long EEMEM persistentChaserEndMinSpeed=0;   // the compiler will give this a unique adress in eeprom, but reading and writing must bedone manually
LongParameterSetting ChaserEndMinSpeedSetting;

//sparkle
long sparkleEndDistance;
long EEMEM persistentSparkleEndDistanced=0;   // the compiler will give this a unique adress in eeprom, but reading and writing must bedone manually
LongParameterSetting SparkleEndDistanceSetting;

enum BlinkingState{
  StandAlone,
  Tusch,
  SingleLight,
  FillUp,
  Chaser,
  Sparkle,
  Strobe
};
class RunningLights{
public:
  int channelStates[RL_N_CHANNELS];
  int channelStatesWritten[RL_N_CHANNELS];
  long channelLastMillis[RL_N_CHANNELS];
  int minStateMillis;  /// channels can change only after so many millis 
  long lastMillis;   ///used by moving lights etc.
  long lastStateChangeMillis;   ///used by moving lights etc.
  BlinkingState state;    //what shall we do when the update is due...
  long chaserOffset;

  int debugLight; ///< -1 is normal operation, positve numbers are lamp indices
  IntParameterSetting debugLightSetting;
  RunningLights(){
    debugLight=-1;
    chaserOffset=0;
    state=StandAlone;
    minStateMillis=10;
    lastMillis=millis();
    lastStateChangeMillis=millis();

    for( int i=0;i<RL_N_CHANNELS;i++){
      channelStates[i]=0;
      channelStatesWritten[i]=channelStates[i];
      channelLastMillis[i]=millis();
      pinMode(RL_CH_INDICES[i], OUTPUT);
      //digitalWrite(RL_CH_INDICES[i], LOW);
    }
  }

  void setup(){
    debugLightSetting.setup(&debugLight,-3,16,F("debug/light"),&oscServer,-1);

    StandAloneChaseDelaySetting.setup(&standAloneChaseDelay,0,1000,F("idle/chaseDelay"),&oscServer,(int)&persistentStandAloneChaseDelay);

    SingleLightEndSpeedSetting.setup(&singleLightEndSpeed,0,50000,F("single/end"),&oscServer,(int)&persistentSingleLightEndSpeed);

    FillUpEndDistanceSetting.setup(&fillUpEndDistance,0,1000,F("fillup/end"),&oscServer,(int)&persistentFillUpEndDistance);
    FillUpFallBackTimeOutSetting.setup(&fillUpFallBackTimeOut,0,10000,F("fillup/timeout"),&oscServer,(int)&persistentFillUpFallBackTimeOut);

    ChaserDistDecaySetting.setup(&chaserDistDecay,0,10000,F("chaser/decay"),&oscServer,(int)&persistentChaserDistDecay);
    ChaserMaxSpeedSetting.setup(&chaserMaxSpeed,0,50000,F("chaser/endSpeed"),&oscServer,(int)&persistentChaserMaxSpeed);
    ChaserEndDistanceSetting.setup(&chaserEndDistance,0,2000,F("chaser/endDist"),&oscServer,(int)&persistentChaserEndDistance);
    ChaserEndMinSpeedSetting.setup(&chaserEndMinSpeed,0,50000,F("chaser/endMinSpeed"),&oscServer,(int)&persistentChaserEndMinSpeed);

  };


  void printState(){
    /*
    for( int i=0;i<RL_N_CHANNELS;i++){
      Serial.print(channelStates[i]);
      Serial.print("\t");
    }*/
    Serial.print ("State: ");
    Serial.print (state);
    Serial.print("\n");

  }
  void writeChannels(){
    //change channel states only once every minStateMillis to save the relais.
    long curMillis=millis();
    for( int i=0;i<RL_N_CHANNELS;i++){
      if(channelStatesWritten[i]!=channelStates[i]){
        long millisPassed=curMillis-channelLastMillis[i];
        if(millisPassed>minStateMillis||millisPassed<0){
          channelLastMillis[i]=curMillis;
          channelStatesWritten[i]=channelStates[i];
        }
      }
      digitalWrite(RL_CH_INDICES[i], channelStatesWritten[i]);
    }
  }
  void updateOuts(Tacho* tacho){
    //prevent trouble from timer overflow
    if(millis()-lastStateChangeMillis<0)lastStateChangeMillis=millis();
    if(millis()-lastMillis<0)lastMillis=millis();

    switch(state){
    case StandAlone:
      if(tacho->lastPeriodTime<500){
        state=Tusch;
        lastStateChangeMillis=millis();
      }
      else{
        generateStandAlone(tacho);
      }
      break;
    case Tusch:
      generateTusch(tacho);
      break;
    case SingleLight:
      generateMovingSingleLight( tacho);
      break;
    case FillUp:
      generateFillUp(tacho);
      break;
    case Chaser:
    generateChaser(tacho);
      break;
    case Sparkle:
      break;
    case Strobe:
      break;
    }

    switch (debugLight){
    case -2:
      for( int i=0;i<RL_N_CHANNELS;i++){
        channelStates[i]=1;
      };
      break;
    case -1:
      //dont do anything
      break;
    default:
      for( int i=0;i<RL_N_CHANNELS;i++){
        channelStates[i]=(i==debugLight);
      }
      break;
    }
    writeChannels();

  };


  void generateMovingSingleLight(Tacho* tacho){
    if(tacho->lastPeriodTime>30000){
      Serial.println("going Idle from SingleLight.");      
      state=StandAlone;
      lastStateChangeMillis=millis();
      return;
    }
    long speed=tacho->smoothedVel;
    int lightIndex=constrain(map(speed,0,singleLightEndSpeed,0,RL_N_CHANNELS),0,RL_N_CHANNELS-1);
    for(int i=0;i<RL_N_CHANNELS;i++){
      channelStates[i]=(i==lightIndex);
    }
    if(speed>singleLightEndSpeed){
      Serial.println("going FillUp from SingleLight.");
      state=FillUp;
      lastStateChangeMillis=millis();
      tacho->accumulatedTurns=0;  
    }


  };

  void generateFillUp(Tacho* tacho){
    if(tacho->lastPeriodTime>fillUpFallBackTimeOut){
      state=SingleLight;
      Serial.println("going SingleLight from Fillup.");
      lastStateChangeMillis=millis();
      return;
    }

    long distance=tacho->accumulatedTurns/1024;
    int lightIndex=constrain(map(distance,0,fillUpEndDistance,0,RL_N_CHANNELS),0,RL_N_CHANNELS-1);
    for(int i=0;i<RL_N_CHANNELS;i++){
      channelStates[i]=(i<=lightIndex);
    }
    
    if(distance>fillUpEndDistance){
    
      Serial.println("going Chaser from FillUp.");
      state=Chaser;
      
      
      
      tacho->accumulatedTurns=0;
      lastStateChangeMillis=millis();
      lastMillis=millis();
      return;
    }
  
  };
  
  void generateChaser(Tacho* tacho){
    long distance=tacho->accumulatedTurns/1024;
    long speed=tacho->smoothedVel;

    if(millis()-lastMillis>10){
      tacho->accumulatedTurns-=chaserDistDecay;   
      chaserOffset=chaserOffset%(1024*1024*512); //prevent overflow
      chaserOffset+=constrain(map(speed,0,chaserMaxSpeed,0,128),0,128);
      lastMillis=millis();
    }
    //chaserOffset changes max. 128*100=12800 per second
    
    if(tacho->accumulatedTurns<0){
      tacho->accumulatedTurns=0;
      state=FillUp;
      Serial.println("going FillUp from Chaser.");
      lastStateChangeMillis=millis();
      return;
    }
    if(tacho->lastPeriodTime>fillUpFallBackTimeOut){
      state=SingleLight;
      Serial.println("going SingleLight from Chaser.");
      lastStateChangeMillis=millis();
      return;
    }
        
    int nChasers=constrain(map(distance,0,chaserEndDistance,1,8),1,8);
    
    for(int i=0;i<RL_N_CHANNELS;i++){
      channelStates[i]=0;
    }
    Serial.print("Chaser Offset: ");
    Serial.print(chaserOffset);
    
    for(int i=0;i<nChasers;i++){
      long pos=chaserOffset/(long)(1024);
      Serial.print("\t pos: ");
      Serial.print(pos);       
      pos+=i;
        Serial.print("\t");
      Serial.print(pos);  
      pos%=RL_N_CHANNELS;
        Serial.print("\t!");
      Serial.print(pos);       
      //=((long)i*(long)2+(chaserOffset/(long)(1024)))%(long)RL_N_CHANNELS;
      //at max speed, pos should change 12800/1024=12.5 positions per second
      channelStates[pos]=1;
    }
    Serial.println(" ");

  }

  void generateStandAlone(Tacho* tacho){
    if(tacho->lastPeriodTime>600000){
      long lightIndex=(millis()/standAloneChaseDelay);
      for(int i=0;i<RL_N_CHANNELS;i++){
        channelStates[i]=(((i-lightIndex)%8 >-2));
      }
    }
    else{
      for(int i=0;i<RL_N_CHANNELS;i++){
        channelStates[i]=1;
      }
    }
  };
  void generateTusch(Tacho* tacho){
    long tuschMillis=millis()-lastStateChangeMillis;
    long idleMillis=millis()-lastMillis;
    for(int o=0;o<5;o++){
      for(int i=0;i<RL_N_CHANNELS;i++){
        channelStates[i]=1;
      }
      writeChannels();
      delay (105);
      for(int i=0;i<RL_N_CHANNELS;i++){
        channelStates[i]=0;
      }
      writeChannels();
      delay (105);
    }
    state=SingleLight;
    lastStateChangeMillis=millis();
    /*
    if(tuschMillis<2500&&idleMillis>random(0,100)){
     int nextLight=random(0,RL_N_CHANNELS);
     channelStates[nextLight]=1;
     lastMillis=millis();
     }
     
     if(tuschMillis>2000&&idleMillis>random(0,200)){
     int nextLight=random(0,RL_N_CHANNELS);
     channelStates[nextLight]=0;
     lastMillis=millis();
     }
     */

  };


};





