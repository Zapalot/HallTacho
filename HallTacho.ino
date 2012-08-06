#define USE_OSC
#define DEBUG 1
#include "Tacho.h"
#include "Blinks.h"
#include <avr/eeprom.h>
#include <ArdParameterSettings.h>
#include <WiFlyHQ.h>
#include <ArdOSCForWiFlyHQ.h>

//the wifi passwd is stored in another file that is not published on github...
#include "WiFiData.h"
extern const char mySSID[];
extern const char myPassword[];
extern const char myDeviceID[]="RadWiFly";
 const char localIP[] = "192.168.2.201";
 const uint16_t localPort =8000;
 const uint16_t remotePort =8001;
 const char remoteHost[]="255.255.255.255";
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

long lastDebugMillis;
void setup(){
  lastDebugMillis=0;
  Serial.begin(115200);
  //set up wifly and osc support
  //setupWiflyComplete(&Serial1);
  //set up wifly and osc support
  configureWiFly(
  &Serial1,
  115200,   
  mySSID,
  myPassword,
  myDeviceID,      ///< for identifacation in the network
  localIP,       ///< a string with numbers, if 0, we will use dhcp to get an ip
  localPort,
  remoteHost,
  remotePort
    );

    tachoInstance.setup();
    relaisControl.setup();
    
  enableDebugOutSetting.setup(&enableDebugOut,0,3,F("debug"),&oscServer,-1);
  dummyStringSetting.setup(dummyString,16,F("dString"),&oscServer,(int)persistentDummyString);
  dumpCallback.setup(&dumpParameterInfos,F("dump"),&oscServer);
  setupTacho();
  delay(1000);
  
};

void loop(){
  updateParametersFromStream(&Serial, 100);
  oscServer.availableCheck();
  tachoInstance.update();
  relaisControl.updateOuts(&tachoInstance);
  if(millis()-lastDebugMillis>100){
    lastDebugMillis=millis();
    if(enableDebugOut&1){
      tachoInstance.printState();
      tachoInstance.sendByOsc(&oscClient);
    }
    if(enableDebugOut&2){
       relaisControl.printState();
       
    }
  }
  
  //delay(50);
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
