#include "Blinks.h"
#include "Tacho.h"
#include <avr/eeprom.h>
#include <ArdParameterSettings.h>
#include <WiFlyHQ.h>
#include <ArdOSCForWiFlyHQ.h>
#include "OscSetup.h"

//because we cannot use function pointers to invoka a method we have to do it the old way...
Tacho tachoInstance;
RunningLights relaisControl;
int EEMEM persistentDummyNumber=0;   // the compiler will give this a unique adress in eeprom, but reading and writing must bedone manually
int dummyNumber;
IntParameterSetting* dummySetting;

char persistentDummyString[16] EEMEM = "testing ...";
char dummyString[16];
StringParameterSetting* dummyStringSetting;



CallbackParameterSetting* dumpCallback;

void setup(){
  Serial.begin(115200);
  //set up wifly and osc support
  setupWiflyComplete(&Serial1);

    
  dummySetting=new IntParameterSetting(&dummyNumber,0,10,F("dummy"),&oscServer,(int)&persistentDummyNumber);
  dummyStringSetting= new StringParameterSetting(dummyString,16,F("dString"),&oscServer,(int)persistentDummyString);
  dumpCallback=new CallbackParameterSetting(&dumpParameterInfos,F("dump"),&oscServer);
  setupTacho();
  delay(1000);
  Serial.println(dummyNumber);
  
};

void loop(){
  updateParametersFromStream(&Serial, 100);
  oscServer.availableCheck();
  tachoInstance.update();
  relaisControl.updateOuts(tachoInstance.smoothedVel);
  /*
  Serial.print(1000.0/(float)tachoInstance.lastPeriodTime);
  Serial.print("\t");
  Serial.print(tachoInstance.curVel);
  Serial.print("\t");
  Serial.print(tachoInstance.smoothedVel);
  Serial.print("\t");
  Serial.println(tachoInstance.lastPeriodTime);
  */
  delay(50);
  //Serial.println(dummyNumber);
};


void setupTacho(){
  pinMode(2, INPUT);
  digitalWrite(2, HIGH);
  attachInterrupt(0,sensorTurnedOn,FALLING);
};

void sensorTurnedOn(){
  tachoInstance.sensorTurnedOn();
};
