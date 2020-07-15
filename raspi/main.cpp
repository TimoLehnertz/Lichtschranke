<<<<<<< HEAD
#include <Arduino.h>
#include <SoftwareSerial.h>
#include <Lichtschranke.h>
#include <SerialCom.h>
#include <utils.h>
#include <WebServer.h>
=======
#include <SerialCom.h>
#include <iostream>
#include <wiringSerial.h>
#include <wiringPi.h>

int main(){
	setup();
	while(loop()){
	}
	return true;
}

void log(const char* log){
	std::cout<<log<<std::endl;
}

uint64_t getNanos(){
	struct timespec now;
	clock_gettime(CLOCK_MONOTONIC_RAW, &now);
	return (uint64_t)now.tv_sec * 1000000000U + (uint64_t)now.tv_nsec;
}

uint64_t getMicros(){
	return getNanos() / 1000;
}

uint64_t getMillis(){
	return getNanos() / 1000000;
}

uint32_t getSeconds(){
    return getNanos() / 1000000000;
}

>>>>>>> ec3ed5cc8d5792cad1ab2a4c298988683230f12f

#define MASTER_PORT 11
#define PING_PORT 12
#define SYNC_PORT 13
#define TRIGGER_PORT 14

<<<<<<< HEAD
void changeSetting(int, int);

unsigned long now; //timing variable updated in loop
=======


unsigned long now; //timing variable updated in loop (delete me)dsfdfs
>>>>>>> ec3ed5cc8d5792cad1ab2a4c298988683230f12f
unsigned int triggers[100];//for master this array acts as a save for triggers. as A member this station saves unsend times in here
int maxTriggerSize = 100;
int triggerSize = 0;

<<<<<<< HEAD
//Libarys
Lichtschranke ls;
SerialCom sCom;
WebServer webServer(triggers, &triggerSize, changeSetting);
SoftwareSerial debug(4, 5);

bool isMaster;
int certifiedMembers[255];//Ports at the master for each specific member
byte certifiedMembersSize;
char chipID[10];
=======


class SInterface : public SerialInterface{
	private:
		int fd;
		void serialBegin(){
			fd = serialOpen("/dev/ttyS0", 9600);
		}
		
	public:
		
		SInterface(){
			serialBegin();
		}
		
        bool available() override{
          return serialDataAvail(fd) > 0 && fd != -1;
        }

        char read() override{
			if(available() && fd != -1)
				return serialGetchar(fd);
			else
				return 0;
        }

        void write(const char* c) override{
			if(fd != -1)
			serialPutchar(fd, c);
        }
};

//Libarys
SInterface si;
SerialCom sCom(&si);
>>>>>>> ec3ed5cc8d5792cad1ab2a4c298988683230f12f
unsigned long pingStart;
unsigned long pingEnd;
bool exec;
bool execRemain = false;
bool isMasterMemberReliable = false;

<<<<<<< HEAD

void changeSetting(int settingID, int value){
  if(settingID == webServer.MIN_COOLDOWN){
    ls.setTimeout(value);
  }
}

//##############################################################################  SyncPart Start  #######################################################################
=======
//##############################################################################  SyncPart Start  #######################################################################

>>>>>>> ec3ed5cc8d5792cad1ab2a4c298988683230f12f
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

<<<<<<< HEAD
/**
 * resets sync parameter and start syncronization
 */
=======

 // resets sync parameter and start syncronization
 
>>>>>>> ec3ed5cc8d5792cad1ab2a4c298988683230f12f
void startSync(){
  isSyncronized = false;//telling not to send timing specific stuff from now on until sync is done
  syncAttemp = 0; //resettin current attempt to 0
  sendSync();
}

bool sendSync(){
<<<<<<< HEAD
  debug.println("start send Sync");
  if(syncAttemp < syncMemorySize){
    syncTimeMember[syncAttemp] = millis();
=======
  log("start send Sync");
  if(syncAttemp < syncMemorySize){
    syncTimeMember[syncAttemp] = getMillis();
>>>>>>> ec3ed5cc8d5792cad1ab2a4c298988683230f12f
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
<<<<<<< HEAD
  debug.println("end send Sync");
=======
  log("end send Sync");
>>>>>>> ec3ed5cc8d5792cad1ab2a4c298988683230f12f
}


void syncCallback(const Com& com){
  int id = indexOf(com.requestID, syncRequestID, syncMemorySize);
  if(com.statusCode == sCom.OK && isNumeric(com.message))
  {
    syncTimeMaster[id] = atoi(com.message);
<<<<<<< HEAD
    syncTimeDuration[id] = millis() - syncTimeMember[id];
=======
    syncTimeDuration[id] = getMillis() - syncTimeMember[id];
>>>>>>> ec3ed5cc8d5792cad1ab2a4c298988683230f12f
    //debug.println("received sync: ID: " + String(id) + ", syncTimeMember:" + String(syncTimeMember[id]) + ", syncTimeMaster: " + String(syncTimeMaster[id]) + ", syncTimeDuration: " + String(syncTimeDuration[id]));
  } else{
    syncAttemp--; //going one attempt back to restard this attemp
  }
  sendSync(); //sending new sync(this only sends when current attemp isnt the last one)
}

<<<<<<< HEAD
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
=======

 // uses the gathered information from the last sync calls to calculate the offset
 
void calcSync(){
  byte quickestindex = getMinIndex(syncTimeDuration, syncMemorySize);
  inaccuracity = quickestindex;
  log("Quickest sync attemp took " + String(syncTimeDuration[quickestindex]) + "ms");
  offset = syncTimeMaster[quickestindex] - (syncTimeMember[quickestindex] + (syncTimeDuration[quickestindex] / 2)) + defaultOffset;
  log("Syncronized.. offset: " + String(offset));
  isSyncronized = true;//telling to be syncronized again
  lastSync = getMillis();
>>>>>>> ec3ed5cc8d5792cad1ab2a4c298988683230f12f
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
<<<<<<< HEAD
    pingEnd = millis();
    debug.println("Ping time from Master: " + String(pingEnd - pingStart));
  } else{
    debug.println("ping timed out :(");
=======
    pingEnd = getMillis();
    log("Ping time from Master: " + String(pingEnd - pingStart));
  } else{
    log("ping timed out :(");
>>>>>>> ec3ed5cc8d5792cad1ab2a4c298988683230f12f
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
<<<<<<< HEAD
  debug.println("Local Trigger!!");
=======
  log("Local Trigger!!");
>>>>>>> ec3ed5cc8d5792cad1ab2a4c298988683230f12f
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
<<<<<<< HEAD
  debug.println("sending trigger: " + String(atTime));
=======
  log("sending trigger: " + String(atTime));
>>>>>>> ec3ed5cc8d5792cad1ab2a4c298988683230f12f
  char* nowChar = getNewCharFromInt(atTime);
  sCom.sendRequest(TRIGGER_PORT, nowChar, triggerRequestReturn);
}

void triggerRequestReturn(const Com& com){
  if(com.statusCode == sCom.OK){//trigger got definetly received
    long message = atoi(com.message);
<<<<<<< HEAD
    debug.println("trigger did arrive :)" + String(message));
    int index = indexOf(message, triggers, triggerSize);
    debug.println("index: " + String(index));
=======
    log("trigger did arrive :)" + String(message));
    int index = indexOf(message, triggers, triggerSize);
    log("index: " + String(index));
>>>>>>> ec3ed5cc8d5792cad1ab2a4c298988683230f12f
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
<<<<<<< HEAD
    debug.println("trigger did not arrive :(");
=======
    log("trigger did not arrive :(");
>>>>>>> ec3ed5cc8d5792cad1ab2a4c298988683230f12f
  }
}

void processIncomingTrigger(unsigned int atTime){
<<<<<<< HEAD
  debug.println("incoming Trigger!!:    " + String(atTime) + "ms    ");
=======
  log("incoming Trigger!!:    " + String(atTime) + "ms    ");
>>>>>>> ec3ed5cc8d5792cad1ab2a4c298988683230f12f
  triggers[triggerSize] = atTime;
  if(triggerSize < maxTriggerSize - 1)
    triggerSize++;
   webServer.updateLiveXML();
}

void sendRemainingTriggers(){
  if(!isMaster && isMasterMemberReliable && ls.isNMsFromLastTrigegr(1500)){
<<<<<<< HEAD
    debug.println("sending remaining triggers! (" + String(triggerSize) + ")");
=======
    log("sending remaining triggers! (" + String(triggerSize) + ")");
>>>>>>> ec3ed5cc8d5792cad1ab2a4c298988683230f12f
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
<<<<<<< HEAD
  sprintf(tmp, "%lu", millis());
=======
  sprintf(tmp, "%lu", getMillis());
>>>>>>> ec3ed5cc8d5792cad1ab2a4c298988683230f12f
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
<<<<<<< HEAD
  debug.print("member \"");debug.print(certifiedMembers[certifiedMembersSize]);debug.println("\" joined :)");
=======
  log("member \"");debug.print(certifiedMembers[certifiedMembersSize]);debug.println("\" joined :)");
>>>>>>> ec3ed5cc8d5792cad1ab2a4c298988683230f12f
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
<<<<<<< HEAD
=======
  log("Member");
>>>>>>> ec3ed5cc8d5792cad1ab2a4c298988683230f12f
  isMaster = false;
  webServer.end();
  //remove Listeners from master
  sCom.removeListener(sCom.REQUEST, MASTER_PORT);
  sCom.removeListener(sCom.REQUEST, PING_PORT);
  sCom.removeListener(sCom.BROADCAST, TRIGGER_PORT);
  //setListeners from Member
  sCom.setBroadcastListener(SYNC_PORT, syncRequestFromMaster);
<<<<<<< HEAD
  pingStart = millis();
=======
  pingStart = getMillis();
>>>>>>> ec3ed5cc8d5792cad1ab2a4c298988683230f12f
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
<<<<<<< HEAD
=======
  log("MasterSetup");
>>>>>>> ec3ed5cc8d5792cad1ab2a4c298988683230f12f
  isMaster = true;
  webServer.begin();
  //remove Listeners from member
  sCom.removeListener(sCom.BROADCAST, SYNC_PORT);
  //set Listeners from master
  sCom.setRequestListener(MASTER_PORT, masterPortRequestCallback);
  sCom.setRequestListener(PING_PORT, masterPingCallback);
  sCom.setRequestListener(TRIGGER_PORT, lsTriggerCallback);
<<<<<<< HEAD
  debug.println("I am Master");
=======
  log("I am Master");
>>>>>>> ec3ed5cc8d5792cad1ab2a4c298988683230f12f
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

<<<<<<< HEAD
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
=======

void setup(){
  //begin functions
  log("debug initialized");
  for (size_t i = 0; i < 3; i++)
  {
    log("booting");
  }
  
  sCom.begin();
  //setup
  sCom.sendRequest(MASTER_PORT, chipID, initRequestToMasterCallback);
>>>>>>> ec3ed5cc8d5792cad1ab2a4c298988683230f12f
}

//###############################################################################  Setup End  ##############################################################

//###############################################################################  Loop Start ##############################################################

<<<<<<< HEAD
void loop(){
  now = millis();
  ls.handleTrigger();
  if(now  % 120000 == 0 && !exec && !isMaster && isMasterMemberReliable){
    pingStart = millis();
=======
bool loop(){
  now = getMillis();
  if(now  % 120000 == 0 && !exec && !isMaster && isMasterMemberReliable){
    pingStart = getMillis();
>>>>>>> ec3ed5cc8d5792cad1ab2a4c298988683230f12f
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
<<<<<<< HEAD
  // if(isMaster && isMasterMemberReliable)
    webServer.handleClient();
}

//###############################################################################  Loop End  ##############################################################
=======
  return true;
}

//###############################################################################  Loop End  ##############################################################
>>>>>>> ec3ed5cc8d5792cad1ab2a4c298988683230f12f
