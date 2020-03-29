// Wemos D1 E1.31 - 8 channel sketch for Sparkfun EL Escudo Dos shield
//Spark Fun link https://www.sparkfun.com/products/10878

#include <ESP8266WiFi.h>
#include <E131.h> // Copyright (c) 2015 Shelby Merrick http://www.forkineye.com

// ***** USER SETUP STUFF *****
const char hostname[] = "GaryRelayTest";
const char ssid[] = "MYSSID"; // replace with your SSID.
const char passphrase[] = "12345678"; // replace with your passphrase.

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

void setup() {
Serial.begin(115200);

  //initialize GPIO pins
  for(int i = 0; i < MAX_CHANNELS; ++i)  {
    pinMode(channels[i], OUTPUT);
    digitalWrite(channels[i], LOW);
  }

  WiFi.hostname(hostname);
/* Choose one to begin listening for E1.31 data */
//e131.begin(ssid, passphrase, ip, netmask, gateway, dns); /* via Unicast on the default port */
e131.beginMulticast(ssid, passphrase, universe, ip, netmask, gateway, dns); /* via Multicast with static IP Address */
//e131.beginMulticast(ssid, passphrase, universe); /* via Multicast with DHCP IP Address */
}

void loop() {
/* Parse a packet */
uint16_t num_channels = e131.parsePacket();

/* Process channel data if we have it */
if (num_channels) {
Serial.println("we have data");

digitalWrite(channels[0], (e131.data[0] > 127) ? HIGH : LOW);
digitalWrite(channels[1], (e131.data[1] > 127) ? HIGH : LOW);
digitalWrite(channels[2], (e131.data[2] > 127) ? HIGH : LOW);
digitalWrite(channels[3], (e131.data[3] > 127) ? HIGH : LOW);
digitalWrite(channels[4], (e131.data[4] > 127) ? HIGH : LOW);
digitalWrite(channels[5], (e131.data[5] > 127) ? HIGH : LOW);
digitalWrite(channels[6], (e131.data[6] > 127) ? HIGH : LOW);
digitalWrite(channels[7], (e131.data[7] > 127) ? HIGH : LOW);
}//end we have data

} // end void loop
