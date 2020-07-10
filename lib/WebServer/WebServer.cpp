#include "WebServer.h"

WebServer::WebServer(unsigned int* times, int* size, settingCallback sc) :
    server(80),
    m_recordedTimes(times),
    m_recordedTimesSize(size),
    apIP(8, 8, 8, 8),
    netMsk(255, 255, 255, 0)
{
    m_settingCallback = sc;
    m_liveCsv[0] ='\0';
}


void WebServer::begin(){
    WiFi.persistent(false);
    if(m_debug){
        m_debug->println("Configuring access point...");
    }
    WiFi.mode(WIFI_AP_STA);
    WiFi.disconnect();
    WiFi.begin(homeSsid, homePasswd);
    WiFi.softAPConfig(apIP, apIP, netMsk);
    WiFi.softAP(softAP_ssid, softAP_password);
    delay(500);
    if(m_debug){
        m_debug->print("AP IP address: ");
        m_debug->println(WiFi.softAPIP());
    }

    /* Setup the DNS server redirecting all the domains to the apIP */
    dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
    dnsServer.start(DNS_PORT, "*", apIP);

    // //pages
    server.on("/wifiroot", std::bind(&WebServer::handleRoot, this));
    server.on("/wifi", std::bind(&WebServer::handleWifi, this));
    server.on("/wifisave", std::bind(&WebServer::handleWifiSave, this));
    server.on("/generate_204", std::bind(&WebServer::handleRoot, this));  //Android captive portal. Maybe not needed. Might be handled by notFound handler.
    server.on("/fwlink", std::bind(&WebServer::handleRoot, this));  //Microsoft captive portal. Maybe not needed. Might be handled by notFound handler.
    server.onNotFound(std::bind(&WebServer::handleNotFound, this));

    //server.on("/reboot", std::bind(&WebServer::reboot, this));
    server.on("/", std::bind(&WebServer::htmlContent, this));
    server.on("/style.css", std::bind(&WebServer::styleContent, this));
    server.on("/live", std::bind(&WebServer::liveContent, this));
    server.on("/settings", std::bind(&WebServer::settings, this));
    //server.on("/export", std::bind(&WebServer::exportFile, this));
    //server.onNotFound(std::bind(&WebServer::handleNotFound, this));
    server.begin();
    if(m_debug){
      m_debug->print("HTTP server started");
    }
    loadCredentials(); // Load WLAN credentials from network
    connect = strlen(ssid) > 0; // Request WLAN connect if there is a SSID
    updateLiveXML();
}

void WebServer::end(){
    server.close();
}

void WebServer::setDebug(SoftwareSerial* debug){
    m_debug = debug;
}

void WebServer::setSettingChangedCallback(settingCallback sc){
    m_settingCallback = sc;
}

void WebServer::handleClient(){
    m_now = millis();
    if (connect) {
        if(m_debug)
            m_debug->println("Connect requested");
        connect = false;
        connectWifi();
        lastConnectTry = millis();
    }
    {
    unsigned int s = WiFi.status();
    if (s == 0 && millis() > (lastConnectTry + 60000)) {
      /* If WLAN disconnected and idle try to connect */
      /* Don't set retry time too low as retry interfere the softAP operation */
      connect = true;
    }
    if (status != s) { // WiFi status change
        if(m_debug){
            m_debug->print("Status: ");
            m_debug->println(s);
        }
      status = s;
      if (s == WL_CONNECTED) {
        /* Just connected to WLAN */
        if(m_debug){
            m_debug->println("");
            m_debug->print("Connected to ");
            m_debug->println(ssid);
            m_debug->print("IP address: ");
            m_debug->println(WiFi.localIP());
        }

        // Setup MDNS responder
        if (!MDNS.begin(myHostname)) {
            if(m_debug)
                 m_debug->println("Error setting up MDNS responder!");
        } else {
            if(m_debug)
                m_debug->println("mDNS responder started");
            // Add service to MDNS-SD
            MDNS.addService("http", "tcp", 80);
        }
      } else if (s == WL_NO_SSID_AVAIL) {
        WiFi.disconnect();
      }
    }
    if (s == WL_CONNECTED) {
      MDNS.update();
    }
  }
  // Do work:
  //DNS
  dnsServer.processNextRequest();
  //HTTP
  server.handleClient();
}

bool WebServer::isIp(String str) {
  for (size_t i = 0; i < str.length(); i++) {
    int c = str.charAt(i);
    if (c != '.' && (c < '0' || c > '9')) {
      return false;
    }
  }
  return true;
}

/** IP to String? */
String WebServer::toStringIp(IPAddress ip) {
  String res = "";
  for (int i = 0; i < 3; i++) {
    res += String((ip >> (8 * i)) & 0xFF) + ".";
  }
  res += String(((ip >> 8 * 3)) & 0xFF);
  return res;
}

/** Load WLAN credentials from EEPROM */
void WebServer::loadCredentials() {
    EEPROM.begin(512);
    if(m_debug)
        m_debug->print("EEPROM size: ");m_debug->println(EEPROM.length());
    EEPROM.get(0, ssid);
    EEPROM.get(0 + sizeof(ssid), password);
    char ok[3];
    EEPROM.get(0 + sizeof(ssid) + sizeof(password), ok);
    EEPROM.end();
    if(m_debug){
        m_debug->print("sizeof(ssid): ");m_debug->println(sizeof(ssid));
        m_debug->print("sizeof(password): ");m_debug->println(sizeof(password));
    }
    if (String(ok) != String("OK")) {
        ssid[0] = 0;
        password[0] = 0;
    }
    if(m_debug){
        m_debug->println("Recovered credentials:");
        m_debug->println(ssid);
        m_debug->println(strlen(password) > 0 ? "********" : "<no password>");
    }
}

/** Store WLAN credentials to EEPROM */
void WebServer::saveCredentials() {
    EEPROM.begin(512);
    EEPROM.put(0, ssid);
    EEPROM.put(0 + sizeof(ssid), password);
    char ok[] = "OK";
    EEPROM.put(0 + sizeof(ssid) + sizeof(password), ok);
    EEPROM.commit();
    EEPROM.end();
}

/** Redirect to captive portal if we got a request for another domain. Return true in that case so the page handler do not try to handle the request again. */
bool WebServer::captivePortal() {
  if (!isIp(server.hostHeader()) && server.hostHeader() != (String(myHostname) + ".local")) {
      if(m_debug)
        m_debug->println("Request redirected to captive portal");
    server.sendHeader("Location", String("http://") + toStringIp(server.client().localIP()), true);
    server.send(302, "text/plain", "");   // Empty content inhibits Content-length header so we have to close the socket ourselves.
    server.client().stop(); // Stop is needed because we sent no content length
    return true;
    }
    return false;
}

void WebServer::handleRoot() {
  if (captivePortal()) { // If caprive portal redirect instead of displaying the page.
    return;
  }
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.send(200, "text/html", "<p>You may want to <a href='/wifi'>config the wifi connection</a>.</p>");
}

/** Wifi config page handler */
void WebServer::handleWifi() {
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");

  String Page;
  Page += F(
            "<html><head></head><body>"
            "<h1>Wifi config</h1>");
  if (server.client().localIP() == apIP) {
    Page += String(F("<p>You are connected through the soft AP: ")) + softAP_ssid + F("</p>");
  } else {
    Page += String(F("<p>You are connected through the wifi network: ")) + ssid + F("</p>");
  }
  Page +=
    String(F(
             "\r\n<br />"
             "<a href='/'>Back</a><table><tr><th align='left'>SoftAP config</th></tr>"
             "<tr><td>SSID ")) +
    String(softAP_ssid) +
    F("</td></tr>"
      "<tr><td>IP ") +
    toStringIp(WiFi.softAPIP()) +
    F("</td></tr>"
      "</table>"
      "\r\n<br />"
      "<table><tr><th align='left'>WLAN config</th></tr>"
      "<tr><td>SSID ") +
    String(ssid) +
    F("</td></tr>"
      "<tr><td>IP ") +
    toStringIp(WiFi.localIP()) +
    F("</td></tr>"
      "</table>"
      "\r\n<br />"
      "<table><tr><th align='left'>WLAN list (refresh if any missing)</th></tr>");
      if(m_debug)
        m_debug->println("scan start");
  int n = WiFi.scanNetworks();
  if(m_debug)
    m_debug->println("scan done");
  if (n > 0) {
    for (int i = 0; i < n; i++) {
      Page += String(F("\r\n<tr><td>SSID ")) + WiFi.SSID(i) + ((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? F(" ") : F(" *")) + F(" (") + WiFi.RSSI(i) + F(")</td></tr>");
    }
  } else {
    Page += F("<tr><td>No WLAN found</td></tr>");
  }
  Page += F(
            "</table>"
            "\r\n<br /><form method='POST' action='wifisave'><h4>Connect to network:</h4>"
            "<input type='text' placeholder='network' name='n'/>"
            "<br /><input type='password' placeholder='password' name='p'/>"
            "<br /><input type='submit' value='Connect/Disconnect'/></form>"
            "<p>You may want to <a href='/'>return to the home page</a>.</p>"
            "</body></html>");
  server.send(200, "text/html", Page);
  server.client().stop(); // Stop is needed because we sent no content length
}

/** Handle the WLAN save form and redirect to WLAN config page again */
void WebServer::handleWifiSave() {
    if(m_debug)
        m_debug->println("wifi save");
    server.arg("n").toCharArray(ssid, sizeof(ssid) - 1);
    server.arg("p").toCharArray(password, sizeof(password) - 1);
    server.sendHeader("Location", "wifi", true);
    server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    server.sendHeader("Pragma", "no-cache");
    server.sendHeader("Expires", "-1");
    server.send(302, "text/plain", "");    // Empty content inhibits Content-length header so we have to close the socket ourselves.
    server.client().stop(); // Stop is needed because we sent no content length
    saveCredentials();
    connect = strlen(ssid) > 0; // Request WLAN connect with new credentials if there is a SSID
}

void WebServer::handleNotFound() {
  if (captivePortal()) { // If caprive portal redirect instead of displaying the error page.
    return;
  }
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.send(404, "text/plain", "The site you requested for does not exist :( I try to redirect you");
}

void WebServer::connectWifi() {
    if(m_debug)
        m_debug->println("Connecting as wifi client...");
    WiFi.disconnect();
    WiFi.begin(ssid, password);
    int connRes = WiFi.waitForConnectResult();
    if(m_debug){
        m_debug->print("connRes: ");
        m_debug->println(connRes);
    }
}

void WebServer::updateLiveXML(){
    char record[11];
    char csvVersionChar[4];
    sprintf(csvVersionChar, "%i", m_csvVersion);
    if(m_debug)
        m_debug->println("csvVersionChar: " + String(csvVersionChar));
    m_liveCsv[0] = 32; //space
    m_liveCsv[1] = 32; //space
    m_liveCsv[2] = 32; //space
    strcpy(m_liveCsv, csvVersionChar);//copy the csv verion into the first 3 spaces
    m_liveCsv[strlen(csvVersionChar)] = 32;
    m_liveCsv[3] = ',';//seperator
    m_liveCsv[m_liveCssSize] = 32;
    for(; m_liveCssValueCount < *m_recordedTimesSize; m_liveCssValueCount++){
        if(m_liveCssSize < m_liveCssMaxSize - 1){
            sprintf(record, "%i", m_recordedTimes[m_liveCssValueCount]);//generating char array from record(nullterminated)
            int size = strlen(record);
            record[size] = ',';//replacing null terminator with comma
            strncpy((m_liveCsv + m_liveCssSize), record, size + 1);
            m_liveCssSize += size + 1;
            if(m_debug){
                m_debug->println("added record: ");
                m_debug->print("m_liveCssValueCount: ");m_debug->println(m_liveCssValueCount);
                m_debug->print("m_recordedTimesSize: ");m_debug->println(*m_recordedTimesSize);
                m_debug->print("m_liveCssSize: ");m_debug->println(m_liveCssSize);
                m_debug->print("strlen(m_liveCsv): ");m_debug->println(strlen(m_liveCsv));
            }
        }
    }
    m_liveCsv[m_liveCssSize] ='\0';
    //if(m_debug)
    //    m_debug->print("liveCsv updated: ");m_debug->println(m_liveCsv);
    m_csvVersion = (m_csvVersion + 1) % 255;
}

//##########################################################################  Pages Start  ###################################################################

void WebServer::htmlContent(){
    if(m_debug)
        m_debug->println("htmlConted got requested");
    server.send(200, "text/html", INDEX_HTML);
    // server.send(200, "text/html", "INDEX_HTML");
}

void WebServer::styleContent(){
    if(m_debug){
        m_debug->println("styleContent got requested");
    }
    /*File css = SPIFFS.open("/style.css", "r");
    server.streamFile(css, "text/css");
    css.close();*/
    server.send(200, "text/css", STYLE_CSS);
    // server.send(200, "text/css", "STYLE_CSS");
}

void WebServer::liveContent(){
    if(m_debug){
        m_debug->println("liveContent got requested");
    }
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "text/csv", m_liveCsv);
}

void WebServer::settings(){
    if(m_debug)
        m_debug->println("settingss got requested");
    if(server.args() > 0){
        server.send(200, "text/plain", "OK");
        if(server.hasArg(MIN_COOLDOWN_ARG)){
            (*m_settingCallback)(MIN_COOLDOWN, server.arg(MIN_COOLDOWN_ARG).toInt());
        }
    } else{
        // server.send(200, "text/html", "SETTINGS_HTML");
        server.send(200, "text/html", SETTINGS_HTML);
    }
}

//##########################################################################  Pages END  ###################################################################