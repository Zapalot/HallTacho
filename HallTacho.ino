#define USE_OSC
#include "Tacho.h"
#include "Blinks.h"
#include <avr/eeprom.h>
#include <ArdParameterSettings.h>
#include <WiFlyHQ.h>
#include <ArdOSCForWiFlyHQ.h>

#include <easyWiFlyOscSetup.h>

//because we cannot use function pointers to invoka a method we have to do it the old way...
Tacho tachoInstance;
RunningLights relaisControl;
int enableDebugOut=0;
IntParameterSetting enableDebugOutSetting;

char persistentDummyString[16] EEMEM = "testing ...";
char dummyString[16];
StringParameterSetting dummyStringSetting;
CallbackParameterSetting dumpCallback;

void setup(){
  Serial.begin(115200);
  //set up wifly and osc support
//  setupWiflyComplete(&Serial1);

    tachoInstance.setup();
    relaisControl.setup();
    
  enableDebugOutSetting.setup(&enableDebugOut,0,1,F("debug"),&oscServer,-1);
  dummyStringSetting.setup(dummyString,16,F("dString"),&oscServer,(int)persistentDummyString);
  dumpCallback.setup(&dumpParameterInfos,F("dump"),&oscServer);
  setupTacho();
  delay(1000);
  
};

void loop(){
  updateParametersFromStream(&Serial, 100);
  //oscServer.availableCheck();
  tachoInstance.update();
  relaisControl.updateOuts(tachoInstance.smoothedVel);
  if(enableDebugOut){
    tachoInstance.printState();
    relaisControl.printState();
  }
  
  delay(50);
  //Serial.println(dummyNumber);
};


void setupTacho(){
  pinMode(2, INPUT_PULLUP);
  digitalWrite(2, HIGH);
  attachInterrupt(0,sensorTurnedOn,FALLING);
};

void sensorTurnedOn(){
  tachoInstance.sensorTurnedOn();
};
