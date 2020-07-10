#ifndef WebServer_h
#define WebServer_h


#include <Arduino.h>
#include <DNSServer.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>
#include <ESP8266WebServer.h>
#include <SoftwareSerial.h>
//#include <FS.h>
#include "style.h"
#include "index.h"
#include "settings.h"

#ifndef APSSID
#define APSSID "AccesPoint"
#define APPSK  "speedFun123"
#endif

#define max(a,b) ((a)>(b)?(a):(b))
#define min(a,b) ((a)<(b)?(a):(b))

typedef void(*settingCallback)(int, int);

class WebServer{
public:
    WebServer(unsigned int* times, int* size, settingCallback sc);
    void constructor(unsigned int*, int*, settingCallback);
    void begin();
    void end();
    void handleClient(); //to put in loop
    void updateLiveXML(); // need to be run when chances took place
    void setDebug(SoftwareSerial*);

    void setSettingChangedCallback(settingCallback);

    const int MIN_COOLDOWN = 0;
    const char* MIN_COOLDOWN_ARG = "minCooldown";

private:
    ESP8266WebServer server;
    SoftwareSerial* m_debug = nullptr;
    bool isIp(String);
    void connectWifi();
    void handleWifiSave();
    void handleWifi();
    bool captivePortal();
    void saveCredentials();
    void loadCredentials();
    void handleRoot();
    String toStringIp(IPAddress);
    //pages
    void flash();
    void terminateFlash();
    void reboot();
    void htmlContent();
    void styleContent();
    void liveContent();
    void settings();
    void exportFile();
    void handleNotFound();

    unsigned long m_now = 0;

    unsigned int* m_recordedTimes = nullptr;
    int* m_recordedTimesSize = nullptr;

    //Server specific constants
    const char *softAP_ssid = APSSID;
    const char *softAP_password = APPSK;
    const char *myHostname = "lichtschranke";

    IPAddress apIP;
    IPAddress netMsk;

    /* Don't set this wifi credentials. They are configurated at runtime and stored on EEPROM */
    char ssid[32];
    char password[32];

    // DNS server
    const byte DNS_PORT = 53;
    DNSServer dnsServer;


    /** Should I connect to WLAN asap? */
    bool connect;

    /** Last time I tried to connect to WLAN */
    unsigned long lastConnectTry = 0;

    /** Current WLAN status */
    unsigned int status = WL_IDLE_STATUS;

    const char* homeSsid = "Lichtschranke";
    const char* homePasswd = "speedFun123";
    //13al07pe19ti58
    //13pe07al19ti58
    
    bool test = 100;

    //dynamicly beeing updated as interface between the sever and client(ajax)
    unsigned int m_csvVersion = 0;
    int m_liveCssSize = 4;
    int m_liveCssValueCount = 0;
    const int m_liveCssMaxSize = 11 * 100;
    char m_liveCsv[11 * 100];

    settingCallback m_settingCallback = nullptr;
};

#endif