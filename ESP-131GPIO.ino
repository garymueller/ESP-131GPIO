// Wemos D1 E1.31 - 8 channel sketch for Sparkfun EL Escudo Dos shield
//Spark Fun link https://www.sparkfun.com/products/10878

#include <ESP8266WiFi.h>
#include <ESPAsyncE131.h>
#include <ESPAsyncWebServer.h>
#include <ESPAsyncDNSServer.h>
#include <FS.h>
#include <ArduinoJson.h>

// ***** USER SETUP STUFF *****
String ssid = "SSID"; // replace with your SSID.
String password = "Password"; // replace with your password.

String CONFIG_FILE = "Config.json";
DynamicJsonDocument Config(1024);
AsyncWebServer server(80);
AsyncDNSServer dnsServer;
ESPAsyncE131 e131;

// Board Pin Definitions
//for Wemos D1 R1 pins are 16,5,4,14,12,13,0,2
//#define MAX_CHANNELS 8
//int channels[MAX_CHANNELS] = {16,5,4,14,12,13,0,2};
//for Wemos D1 R2 pins are 16,5,4,0,2,14,12,13
#define MAX_CHANNELS 8
int channels[MAX_CHANNELS] = {16,5,4,0,2,14,12,13};

//Forward Declarations
String WebReplace(const String& var);

bool LoadConfig();
void SaveConfig(AsyncWebServerRequest* request);

void InitWifi();
void Init131();
void InitWeb();

/*
 * Sets up the initial state and starts sockets
 */
void setup() {
  Serial.begin(115200);

  //initialize GPIO pins
  for(int i = 0; i < MAX_CHANNELS; ++i)  {
    pinMode(channels[i], OUTPUT);
    digitalWrite(channels[i], LOW);
  }
  
  LoadConfig();

  InitWifi();

  Init131();

  InitWeb();
}

/*
 * Main Event Loop
 */
void loop() {
  e131_packet_t packet;
  bool Digital = Config["GPIO"]["digital"].as<bool>();
  int DigitalThreshold = Config["GPIO"]["digital_threshold"].as<int>();
  int ChannelOffset = Config["E131"]["channel_offset"].as<int>();

  while(!e131.isEmpty()) {
    e131.pull(&packet);     // Pull packet from ring buffer
    uint16_t num_channels = htons(packet.property_value_count) - 1;
    for(int i = 0;i < MAX_CHANNELS && i < num_channels; ++i) {
      //seems odd that the array index is 1 based, not zero based ... but it works
      uint16_t data = packet.property_values[ChannelOffset+i+1];
      if(Digital) {
        digitalWrite(channels[i], (data >= DigitalThreshold) ? HIGH : LOW);
      } else {
        //pwm board is 10 bit (1024), data is 256
        analogWrite(channels[i], data*4);
      }
      //Serial.printf("%d:%d:%d; ",i,data,(data >= DigitalThreshold) ? HIGH : LOW);
    }//for loop
    //Serial.println("");
  }//! empty
}// end void loop 




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
    Config["network"]["static"] = false;
    Config["network"]["static_ip"] = "192.168.1.100";
    Config["network"]["static_netmask"] = "255.255.255.0";
    Config["network"]["static_gateway"] = "192.168.1.1";
    Config["network"]["access_point"] = true;
    
    Config["E131"]["multicast"] = "true";
    Config["E131"]["universe"] = 1;
    Config["E131"]["channel_offset"] = 0;

    Config["GPIO"]["digital"] = "true";
    Config["GPIO"]["digital_threshold"] = 127;
    
  } else {
    Serial.println("Loading Configuration File");
    deserializeJson(Config, file);
  }

  serializeJson(Config, Serial);Serial.println();
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

  Config["network"]["hostname"] = request->getParam("hostname",true)->value();
  Config["network"]["ssid"] = request->getParam("ssid",true)->value();
  Config["network"]["password"] = request->getParam("password",true)->value();
  //checkbox status isnt always included if toggled off
  Config["network"]["static"] = 
    (request->hasParam("static",true) && (request->getParam("static",true)->value() == "on"));
  Config["network"]["static_ip"] = request->getParam("static_ip",true)->value();
  Config["network"]["static_netmask"] = request->getParam("static_netmask",true)->value();
  Config["network"]["static_gateway"] = request->getParam("static_gateway",true)->value();
  Config["network"]["access_point"] = 
    (request->hasParam("access_point",true) && (request->getParam("access_point",true)->value() == "on"));
  
  //checkbox status isnt always included if toggled off
  Config["E131"]["multicast"] = 
    (request->hasParam("multicast",true) && (request->getParam("multicast",true)->value() == "on"));
  Config["E131"]["universe"] = request->getParam("universe",true)->value();
  Config["E131"]["channel_offset"] = request->getParam("channel_offset",true)->value();

  //checkbox status isnt always included if toggled off
  Config["GPIO"]["digital"] =
    (request->hasParam("digital",true) && (request->getParam("digital",true)->value() == "on"));
  Config["GPIO"]["digital_threshold"] = request->getParam("digital_threshold",true)->value();

  File file = SPIFFS.open(CONFIG_FILE, "w");
  if(file) {
    serializeJson(Config, file);
    serializeJson(Config, Serial);
  }

  request->send(200);
  ESP.restart();
}

/*
 * Initialiaze Wifi (DHCP/STATIC and Access Point)
 */
void InitWifi()
{
  // Switch to station mode and disconnect just in case
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  
  WiFi.hostname(Config["network"]["hostname"].as<const char*>());

  //configure Static/DHCP
  if(Config["network"]["static"].as<bool>()) {
    IPAddress IP; IP.fromString(Config["network"]["static_ip"].as<String>());
    IPAddress Netmask; Netmask.fromString(Config["network"]["static_netmask"].as<String>());
    IPAddress Gateway; Gateway.fromString(Config["network"]["static_gateway"].as<String>());

    if(WiFi.config(IP, Netmask, Gateway)) {
      Serial.println("Successfully configured static IP");
    } else {
      Serial.println("Failed to configure static IP");
    }
  } else {
    Serial.println("Connecting with DHCP");
  }

  //Connect
  int Timeout = 15000;
  WiFi.begin(Config["network"]["ssid"].as<String>(), Config["network"]["password"].as<String>());
  if(WiFi.waitForConnectResult(Timeout) != WL_CONNECTED) {
    if(Config["network"]["access_point"].as<bool>()) {
      Serial.println("*** FAILED TO ASSOCIATE WITH AP, GOING SOFTAP ***");
      WiFi.mode(WIFI_AP);
      WiFi.softAP(Config["network"]["hostname"].as<String>());
      dnsServer.start(53, "*", WiFi.softAPIP());
    } else {
      Serial.println(F("*** FAILED TO ASSOCIATE WITH AP, REBOOTING ***"));
      ESP.restart();
    }
  } else {
    Serial.printf("Connected as %s\n",WiFi.localIP().toString().c_str());
  }

  WiFi.printDiag(Serial);
}

void Init131()
{
  if(Config["E131"]["multicast"].as<bool>()) {
    e131.begin(E131_MULTICAST, Config["E131"]["universe"]);
  } else {
    e131.begin(E131_UNICAST);
  }
}


/*
 * Initializes the webserver
 */
void InitWeb()
{
  //enables redirect to /index.html on AP connection
  server.onNotFound([](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", String(), false, WebReplace);
  });

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", String(), false, WebReplace);
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

  server.begin();
}

/*
 * WebReplace:
 * Substitutes variables inside of the html
 */
String WebReplace(const String& var)
{
  //Status Page
  if (var == "SSID") {
    return (String)WiFi.SSID();
  } else if (var == "HOSTNAME") {
    return (String)WiFi.hostname();
  } else if (var == "IP") {  
    if(WiFi.getMode() == WIFI_AP) {
      return WiFi.softAPIP().toString();
    } else {
      return WiFi.localIP().toString();
    }
  } else if (var == "MAC") {
    return (String)WiFi.macAddress();
  } else if (var == "RSSI") {
    return (String)WiFi.RSSI();
  } else if (var == "HEAP") {
    return (String)ESP.getFreeHeap();
  } else if (var == "UPTIME") {
    return String(millis());
  } else if (var == "UNIVERSE") {
    return Config["E131"]["universe"];
  } else if (var == "PACKETS") {
    return (String)e131.stats.num_packets;
  } else if (var == "PACKET_ERR") {
    return (String)e131.stats.packet_errors;
  } else if (var == "LAST_IP") {
    return e131.stats.last_clientIP.toString();

  //Configuration Page   
  } else if (var == "CONFIG_HOSTNAME") {
    return Config["network"]["hostname"];
  } else if (var == "CONFIG_SSID") {
    return Config["network"]["ssid"];
  } else if (var == "CONFIG_PASSWORD") {
    return Config["network"]["password"];
  } else if (var == "CONFIG_AP") {
    if(Config["network"]["access_point"].as<bool>())
      return "checked";
     else
      return "";
  } else if (var == "CONFIG_STATIC") {
    if(Config["network"]["static"].as<bool>())
      return "checked";
     else
      return "";
  } else if (var == "CONFIG_STATIC_IP") {
    return Config["network"]["static_ip"];
  } else if (var == "CONFIG_STATIC_NETMASK") {
    return Config["network"]["static_netmask"];
  } else if (var == "CONFIG_STATIC_GATEWAY") {
    return Config["network"]["static_gateway"];
  } else if (var == "CONFIG_MULTICAST") {
    if(Config["E131"]["multicast"].as<bool>())
      return "checked";
     else
      return "";
  } else if (var == "CONFIG_UNIVERSE") {
    return Config["E131"]["universe"];
  } else if (var == "CONFIG_CHANNEL_OFFSET") {
    return Config["E131"]["channel_offset"];
  } else if (var == "CONFIG_DIGITAL") {
    if(Config["GPIO"]["digital"].as<bool>())
      return "checked";
     else
      return "";
  } else if (var == "CONFIG_THRESHOLD") {
    return Config["GPIO"]["digital_threshold"];
  
  //Relay Page
  } else if (var == "RELAYS") {
    String Relays = "";
    for(int i = 0; i < MAX_CHANNELS; ++i) {
      Relays += "<label>Relay "+String(i+1)+"</label>";
      Relays += "  <label class=\"switch\">";
      Relays += "  <input type=\"checkbox\" ";
      if(digitalRead(channels[i]) == HIGH) {
        Relays += "checked";
      }
      Relays += " onclick=\"fetch('SetRelay?relay="+String(i)+"&checked='+this.checked);\">";
      Relays += "  <span class=\"slider round\"></span>";
      Relays += "</label>";
      Relays += "<br><br>";
    }
    return Relays;
  }
 
  return var;
}
