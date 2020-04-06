// Wemos D1 E1.31 - 8 channel sketch for Sparkfun EL Escudo Dos shield
//Spark Fun link https://www.sparkfun.com/products/10878

#include <ESP8266WiFi.h>
#include <E131.h> // Copyright (c) 2015 Shelby Merrick http://www.forkineye.com
#include <ESPAsyncWebServer.h>
#include <FS.h>
#include <ArduinoJson.h>

String CONFIG_FILE = "Config.json";

// ***** USER SETUP STUFF *****
String ssid = "SSID"; // replace with your SSID.
String password = "Password"; // replace with your password.

AsyncWebServer server(80);
//AsyncDNSServer dns;

StaticJsonDocument<2048> Config;

const int universe = 12; // this sets the universe number you are using.
//The IP setup is only required if you are using Unicast and/or you want a static IP for multicast.
//In multicast this IP is only for network connectivity not multicast data
IPAddress ip (192,168,1,222); // xx,xx,xx,xx
IPAddress netmask (255,255,255,0); //255,255,255,0 is common
IPAddress gateway (192,168,1,1); // xx,xx,xx,xx normally your router / access piont IP address
IPAddress dns (192,168,1,1); // // xx,xx,xx,xx normally your router / access point IP address

// this sets the pin numbers to use as outputs.
//for Wemos D1 R1 pins are 16,5,4,14,12,13,0,2
//for Wemos D1 R2 pins are 16,5,4,0,2,14,12,13
#define MAX_CHANNELS 8
int channels[MAX_CHANNELS] = {16,5,4,0,2,14,12,13};

E131 e131;

//Forward Declarations
String processor(const String& var);
bool LoadConfig();
void SaveConfig(AsyncWebServerRequest* request);


/*
 * Sets up the initial state and starts sockets
 */
void setup() {
  Serial.begin(115200);

  LoadConfig();

  //initialize GPIO pins
  for(int i = 0; i < MAX_CHANNELS; ++i)  {
    pinMode(channels[i], OUTPUT);
    digitalWrite(channels[i], LOW);
  }

  WiFi.hostname(Config["network"]["hostname"].as<const char*>());
/* Choose one to begin listening for E1.31 data */
//e131.begin(ssid, passphrase);
//e131.begin(ssid, passphrase, ip, netmask, gateway, dns); /* via Unicast on the default port */
//e131.beginMulticast(ssid, passphrase, universe, ip, netmask, gateway, dns); /* via Multicast with static IP Address */
e131.beginMulticast(Config["network"]["ssid"], Config["network"]["password"], Config["E131"]["universe"]); /* via Multicast with DHCP IP Address */

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });
  server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/favicon.png", "image/png");
  });
  server.on("/SaveConfig", HTTP_POST, [](AsyncWebServerRequest *request){
    SaveConfig(request);
  });
  server.on("/SetRelay", HTTP_GET, [](AsyncWebServerRequest *request){
    int relay = request->getParam("relay")->value().toInt();
    if(relay<0 || relay >= MAX_CHANNELS) {
      Serial.println("SetRelay - Index out of range");
      return;
    }
    digitalWrite(channels[relay], (request->getParam("checked")->value() == "true") ? HIGH : LOW);
    request->send(200);
  });

  // Start server
  server.begin();
}

/*
 * Main Event Loop
 */
void loop() {
  /* Parse a packet */
  uint16_t num_channels = e131.parsePacket();

  /* Process channel data if we have it */
  if (num_channels) {
    Serial.println("we have data");
    for(int i = 0;i < MAX_CHANNELS; ++i) {
      digitalWrite(channels[i], (e131.data[i] > 127) ? HIGH : LOW);
    }
  }//end we have data
} // end void loop


/*
 * processor:
 * Substitutes variables inside of the html
 */
String processor(const String& var)
{
  if (var == "SSID") {
    return (String)WiFi.SSID();
  } else if (var == "HOSTNAME") {
    return (String)WiFi.hostname();
  } else if (var == "IP") {
    return WiFi.localIP().toString();
  } else if (var == "MAC") {
    return (String)WiFi.macAddress();
  } else if (var == "RSSI") {
    return (String)WiFi.RSSI();
  } else if (var == "HEAP") {
    return (String)ESP.getFreeHeap();
  } else if (var == "UPTIME") {
    long day = 86400000; // 86400000 milliseconds in a day
    long hour = 3600000; // 3600000 milliseconds in an hour
    long minute = 60000; // 60000 milliseconds in a minute
    long second =  1000; // 1000 milliseconds in a second
    long timeNow = millis();
   
    int days = timeNow / day ;                                //number of days
    int hours = (timeNow % day) / hour;                       //the remainder from days division (in milliseconds) divided by hours, this gives the full hours
    int minutes = ((timeNow % day) % hour) / minute ;         //and so on...
    int seconds = (((timeNow % day) % hour) % minute) / second;

    String uptime;
    uptime += String(days) + " days, ";
    uptime += String(hours) + ":";
    uptime += String(minutes) + ":";
    uptime += String(seconds);
    return uptime;
  } else if (var == "UNIVERSE") {
    return Config["E131"]["universe"];
  } else if (var == "PACKETS") {
    return (String)e131.stats.num_packets;
  } else if (var == "PACKET_ERR") {
    return (String)e131.stats.packet_errors;
  } else if (var == "LAST_IP") {
    return e131.stats.last_clientIP.toString();
  } else if (var == "RELAYS") {
    String Relays = "";
    for(int i = 0; i < MAX_CHANNELS; ++i) {
      Relays += "<label>Relay "+String(i+1)+"</label>";
      Relays += "  <label class=\"switch\">";
      Relays += "  <input type=\"checkbox\" onclick=\"fetch('SetRelay?relay="+String(i)+"&checked='+this.checked);\">";
      Relays += "  <span class=\"slider round\"></span>";
      Relays += "</label>";
      Relays += "<br><br>";
    }
    return Relays;   
  } else if (var == "CONFIG_HOSTNAME") {
    return Config["network"]["hostname"];
  } else if (var == "CONFIG_SSID") {
    return Config["network"]["ssid"];
  } else if (var == "CONFIG_PASSWORD") {
    return Config["network"]["password"];
  } else if (var == "CONFIG_MULTICAST") {
    if(Config["E131"]["multicast"].as<bool>())
      return "checked";
     else
      return "";
  } else if (var == "CONFIG_UNIVERSE") {
    return Config["E131"]["universe"];
  } 
 
  return var;
}

/*
 * Loads the config from SPPS into the Config variable
 */
bool LoadConfig()
{
  // Initialize SPIFFS
  if(!SPIFFS.begin()){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return false;
  }

  File file = SPIFFS.open(CONFIG_FILE, "r");
  if (!file) {
    //First Run, setup defaults
    Config["network"]["hostname"] = "esps-" + String(ESP.getChipId(), HEX);
    Config["network"]["ssid"] = ssid;
    Config["network"]["password"] = password;

    Config["E131"]["multicast"] = "true";
    Config["E131"]["universe"] = 1;
    
  } else {
    Serial.println("Loading Configuration File");
    deserializeJson(Config, file);
    serializeJson(Config, Serial);
  }
}

void SaveConfig(AsyncWebServerRequest* request)
{
  Serial.println("Saving Configuration File");
  
  //debug
  int params = request->params();
  for(int i=0;i<params;i++){
    AsyncWebParameter* p = request->getParam(i);
    Serial.printf("Param: %s, %s\n", p->name().c_str(), p->value().c_str());
  }

  //Validate Config

  Config["network"]["hostname"] = request->getParam("hostname",true)->value().c_str();
  Config["network"]["ssid"] = request->getParam("ssid",true)->value().c_str();
  Config["network"]["password"] = request->getParam("password",true)->value().c_str();

  Config["E131"]["multicast"] = (request->getParam("multicast",true)->value() == "on") ? true : false;
  Config["E131"]["universe"] = request->getParam("universe",true)->value().toInt();

  File file = SPIFFS.open(CONFIG_FILE, "w");
  if(file) {
    serializeJson(Config, file);
    serializeJson(Config, Serial);
  }

  request->send(200);
  ESP.restart();
}
