#include <Arduino.h>
#include <SoftwareSerial.h>
#include <Lichtschranke.h>
#include <SerialCom.h>
#include <utils.h>
#include <WebServer.h>

#define MASTER_PORT 11
#define PING_PORT 12
#define SYNC_PORT 13
#define TRIGGER_PORT 14

void changeSetting(int, int);

unsigned long now; //timing variable updated in loop (delete me)dsfdfs
unsigned int triggers[100];//for master this array acts as a save for triggers. as A member this station saves unsend times in here
int maxTriggerSize = 100;
int triggerSize = 0;

//Libarys
Lichtschranke ls;
SerialCom sCom;
WebServer webServer(triggers, &triggerSize, changeSetting);
SoftwareSerial debug(4, 5);

bool isMaster;
int certifiedMembers[255];//Ports at the master for each specific member
byte certifiedMembersSize;
char chipID[10];
unsigned long pingStart;
unsigned long pingEnd;
bool exec;
bool execRemain = false;
bool isMasterMemberReliable = false;


void changeSetting(int settingID, int value){
  if(settingID == webServer.MIN_COOLDOWN){
    ls.setTimeout(value);
  }
}

//##############################################################################  SyncPart Start  #######################################################################
void calcSync();
bool sendSync();
void syncCallback(const Com&);
void startSync();
//Data
long offset;
int inaccuracity;
bool isSyncronized = false;
int syncMemorySize = 3;
byte syncAttemp = 0;
long syncTimeMember[3];
long syncTimeMaster[3];
int syncTimeDuration[3];
int syncRequestID[3];
long lastSync = 0;
const int defaultOffset = 8;

/**
 * resets sync parameter and start syncronization
 */
void startSync(){
  isSyncronized = false;//telling not to send timing specific stuff from now on until sync is done
  syncAttemp = 0; //resettin current attempt to 0
  sendSync();
}

bool sendSync(){
  debug.println("start send Sync");
  if(syncAttemp < syncMemorySize){
    syncTimeMember[syncAttemp] = millis();
    char* tmp = new char[1];
    tmp[0] = 0;
    syncRequestID[syncAttemp] = sCom.sendRequest(PING_PORT, tmp, syncCallback);
    //debug.println("sending sync: " + String(syncAttemp) + ". sync request. ID: " + String(syncRequestID[syncAttemp]));
    syncAttemp++;
    return true;
  } else{
    syncAttemp = 0; //resettin current attempt to 0
    calcSync();
    return false;
  }
  debug.println("end send Sync");
}


void syncCallback(const Com& com){
  int id = indexOf(com.requestID, syncRequestID, syncMemorySize);
  if(com.statusCode == sCom.OK && isNumeric(com.message))
  {
    syncTimeMaster[id] = atoi(com.message);
    syncTimeDuration[id] = millis() - syncTimeMember[id];
    //debug.println("received sync: ID: " + String(id) + ", syncTimeMember:" + String(syncTimeMember[id]) + ", syncTimeMaster: " + String(syncTimeMaster[id]) + ", syncTimeDuration: " + String(syncTimeDuration[id]));
  } else{
    syncAttemp--; //going one attempt back to restard this attemp
  }
  sendSync(); //sending new sync(this only sends when current attemp isnt the last one)
}

/**
 * uses the gathered information from the last sync calls to calculate the offset
 */
void calcSync(){
  byte quickestindex = getMinIndex(syncTimeDuration, syncMemorySize);
  inaccuracity = quickestindex;
  debug.println("Quickest sync attemp took " + String(syncTimeDuration[quickestindex]) + "ms");
  offset = syncTimeMaster[quickestindex] - (syncTimeMember[quickestindex] + (syncTimeDuration[quickestindex] / 2)) + defaultOffset;
  debug.println("Syncronized.. offset: " + String(offset));
  isSyncronized = true;//telling to be syncronized again
  lastSync = millis();
}

unsigned long getMasterTime(unsigned long now){
  if(isSyncronized)
    return 0;
  else
    return now + offset;
}

//##############################################################################  SyncPart End  ################################################################################

void memberPongCallback(const Com& com){
  if(com.statusCode == sCom.OK){
    pingEnd = millis();
    debug.println("Ping time from Master: " + String(pingEnd - pingStart));
  } else{
    debug.println("ping timed out :(");
  }
}

bool sendSyncRequest(){
  if(isMasterMemberReliable && isMaster){
    char* tmp = new char[1];
    tmp[0] = 0;
    sCom.sendBroadcast(SYNC_PORT, tmp);
  }
  return isMasterMemberReliable && isMaster;
}

//###############################################################################  Timing Start  ##############################################################

void processIncomingTrigger(unsigned int atTime);
void triggerRequestReturn(const Com& com);
void sendTrigger(long atLocalTime);

void processLocalTrigger(unsigned int now){
  debug.println("Local Trigger!!");
  if(isMasterMemberReliable && isMaster){ //Master
    processIncomingTrigger(now);
  }
  if(isMasterMemberReliable && !isMaster){//Member
    sendTrigger(now + offset);
    triggers[triggerSize] = now + offset;
    triggerSize++;
  }
}

void sendTrigger(long atTime){
  debug.println("sending trigger: " + String(atTime));
  char* nowChar = getNewCharFromInt(atTime);
  sCom.sendRequest(TRIGGER_PORT, nowChar, triggerRequestReturn);
}

void triggerRequestReturn(const Com& com){
  if(com.statusCode == sCom.OK){//trigger got definetly received
    long message = atoi(com.message);
    debug.println("trigger did arrive :)" + String(message));
    int index = indexOf(message, triggers, triggerSize);
    debug.println("index: " + String(index));
    if(index > -1){
      if(index == triggerSize - 1){
        triggerSize--;
        if(triggerSize > 0){
          int tmp = triggers[triggerSize - 1];
          while(tmp == 0){
            triggerSize--;
            if(triggerSize == 0)
              break;
            tmp = triggers[triggerSize - 1];
          }
        }
      }
      triggers[index] = 0;
    }
  } else{//trigger did not arrived
    debug.println("trigger did not arrive :(");
  }
}

void processIncomingTrigger(unsigned int atTime){
  debug.println("incoming Trigger!!:    " + String(atTime) + "ms    ");
  triggers[triggerSize] = atTime;
  if(triggerSize < maxTriggerSize - 1)
    triggerSize++;
   webServer.updateLiveXML();
}

void sendRemainingTriggers(){
  if(!isMaster && isMasterMemberReliable && ls.isNMsFromLastTrigegr(1500)){
    debug.println("sending remaining triggers! (" + String(triggerSize) + ")");
    for (int i = 0; i < triggerSize; i++){
      if(triggers[i] > 0){
        sendTrigger(triggers[i]);
      }
    }
  }
}

//###############################################################################  Timing End  ##############################################################

//###############################################################################  Member Ports Start  ##############################################################

void syncRequestFromMaster(const Com& com){
  if(com.statusCode == sCom.OK && !isMaster && isMasterMemberReliable){
    startSync();
  }
}

//###############################################################################  Member Ports Start  ##############################################################

//###############################################################################  Master Ports Start  ##############################################################

char* lsTriggerCallback(const Com& com){
  char* tmp = new char[10];
  tmp[0] = 0;
  if(com.statusCode == sCom.OK && isMaster && isMasterMemberReliable && isNumeric(com.message)){
    processIncomingTrigger(atoi(com.message));
    strcpy(tmp, com.message);
    return tmp;
  } else{
    return tmp;
  }
}

char* masterPingCallback(const Com& com){
  char* tmp = new char[10];
  sprintf(tmp, "%lu", millis());
  return tmp;
}

/**
 * callback for requests comming from members to master
 * used for checking if a master is available
 * this callback is only allowed to be used by masters
 * it accepts a unice name as message this name will be certiffied and allowed to be used
 */
char* masterPortRequestCallback(const Com& com){
  certifiedMembers[certifiedMembersSize] = atoi(com.message); //certifieing this member
  debug.print("member \"");debug.print(certifiedMembers[certifiedMembersSize]);debug.println("\" joined :)");
  certifiedMembersSize++;
  char* out = new char[2];
  out[0] = 'H'; out[1] = 0;
  return out;
}

//###############################################################################  Master Ports Start  ##############################################################

//###############################################################################  Setup  ##############################################################

/**
 * sets this station up as Member
 * should undo all relevant changes made by the masterSetup()
*/
void memberSetup(){
  isMaster = false;
  webServer.end();
  //remove Listeners from master
  sCom.removeListener(sCom.REQUEST, MASTER_PORT);
  sCom.removeListener(sCom.REQUEST, PING_PORT);
  sCom.removeListener(sCom.BROADCAST, TRIGGER_PORT);
  //setListeners from Member
  sCom.setBroadcastListener(SYNC_PORT, syncRequestFromMaster);
  pingStart = millis();
  debug.println("I am Member");
  certifiedMembersSize = 0;
  isMasterMemberReliable = true;
  startSync();
  triggerSize = 0;
}

/**
 * sets this station up as Master
 * should undo all relevant changes made by the memberSetup()
*/
void masterSetup(){
  isMaster = true;
  webServer.begin();
  //remove Listeners from member
  sCom.removeListener(sCom.BROADCAST, SYNC_PORT);
  //set Listeners from master
  sCom.setRequestListener(MASTER_PORT, masterPortRequestCallback);
  sCom.setRequestListener(PING_PORT, masterPingCallback);
  sCom.setRequestListener(TRIGGER_PORT, lsTriggerCallback);
  debug.println("I am Master");
  certifiedMembersSize = 0;
  isMasterMemberReliable = true;
  sendSyncRequest();//cause sync on all members
  triggerSize = 0;
}

/**
 * Sends a request to the master_port and checks if it gets an answer
 * if no returns 0(false)
 * if yes it returnes the port it got as answer from the master
 */
void initRequestToMasterCallback(const Com& com){
  if(com.statusCode == sCom.OK){ // Master did respond
    memberSetup();
  } else { //master did not respond
    masterSetup();
  }
}

/**
 * Sets the name variable to this chips ID
 */
void setUniqueName(){
  sprintf(chipID, "%i", ESP.getChipId());
}

void setup(){
  //begin functions
  Serial.begin(9600);
  debug.begin(9600);
  debug.println("debug initialized");
  for (size_t i = 0; i < 5; i++)
  {
    debug.println("booting");
  }
  
  ls.begin();
  sCom.begin();
  //debug
  //sCom.setDebug(debug);
  webServer.setDebug(&debug);
  //setup
  setUniqueName();
  sCom.sendRequest(MASTER_PORT, chipID, initRequestToMasterCallback);
  ls.setTriggerCallback(processLocalTrigger);
}

//###############################################################################  Setup End  ##############################################################

//###############################################################################  Loop Start ##############################################################

void loop(){
  now = millis();
  ls.handleTrigger();
  if(now  % 120000 == 0 && !exec && !isMaster && isMasterMemberReliable){
    pingStart = millis();
    sCom.sendRequest(PING_PORT, "ping", memberPongCallback);
    startSync();
    exec = true;
  } else if(now  % 1000 != 0){
    exec = false;
  }
  if(now % 5000 == 0 && !execRemain){
    sendRemainingTriggers();
    execRemain = true;
  } if(now % 5000 != 0)
    execRemain = false;
  
  sCom.handleComunication();
  // if(isMaster && isMasterMemberReliable)
    webServer.handleClient();
}

//###############################################################################  Loop End  ##############################################################