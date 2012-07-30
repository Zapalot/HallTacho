#include "Arduino.h"
#define RL_N_CHANNELS 16
const int RL_CH_INDICES[]= {22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37};

class RunningLights{
public:
  int channelStates[RL_N_CHANNELS];
  
  RunningLights(){
    for( int i=0;i<RL_N_CHANNELS;i++){
      channelStates[i]=0;
      pinMode(RL_CH_INDICES[i], OUTPUT);
      digitalWrite(RL_CH_INDICES[i], LOW);
    }
  }
  
  void setup(){
  
  };

  void writeChannels(){
    for( int i=0;i<RL_N_CHANNELS;i++){
      digitalWrite(RL_CH_INDICES[i], !channelStates[i]);
    }
  }
  void updateOuts(long speed){
    writeMovingSingleLight(map(speed,0,2000,0,1000));
  };

  void writeMovingSingleLight(long permille){
    int lightIndex=constrain(map(permille,0,1000,0,RL_N_CHANNELS),0,RL_N_CHANNELS-1);
    for(int i=0;i<RL_N_CHANNELS;i++){
      channelStates[i]=(i==lightIndex);
    }
    writeChannels();
  };
  

};
