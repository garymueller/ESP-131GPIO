// Wemos D1 E1.31 - 8 channel sketch for Sparkfun EL Escudo Dos shield
//Spark Fun link https://www.sparkfun.com/products/10878

#include <ESP8266WiFi.h>
#include <E131.h> // Copyright (c) 2015 Shelby Merrick http://www.forkineye.com

// ***** USER SETUP STUFF *****
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
const int output_1 = 16; //the pin to use as output 1 (D0)
const int output_2 = 5; //the pin to use as output 2 (D1)
const int output_3 = 4; //the pin to use as output 3 (D2)
const int output_4 = 0; //the pin to use as output 4 (D3)
const int output_5 = 2; //the pin to use as output 5 (D4)
const int output_6 = 14; //the pin to use as output 6 (D5)
const int output_7 = 12; //the pin to use as output 7 (D6)
const int output_8 = 13; //the pin to use as output 8 (D7)


E131 e131;

void setup() {
Serial.begin(115200);
// set the pins chosen above as outputs.
pinMode(output_1, OUTPUT);
pinMode(output_2, OUTPUT);
pinMode(output_3, OUTPUT);
pinMode(output_4, OUTPUT);
pinMode(output_5, OUTPUT);
pinMode(output_6, OUTPUT);
pinMode(output_7, OUTPUT);
pinMode(output_8, OUTPUT);

// set the pins chosen above to low / off.
digitalWrite(output_1, LOW);
digitalWrite(output_2, LOW);
digitalWrite(output_3, LOW);
digitalWrite(output_4, LOW);
digitalWrite(output_5, LOW);
digitalWrite(output_6, LOW);
digitalWrite(output_7, LOW);
digitalWrite(output_8, LOW);

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

digitalWrite(output_1, (e131.data[0] > 127) ? HIGH : LOW);
digitalWrite(output_2, (e131.data[1] > 127) ? HIGH : LOW);
digitalWrite(output_3, (e131.data[2] > 127) ? HIGH : LOW);
digitalWrite(output_4, (e131.data[3] > 127) ? HIGH : LOW);
digitalWrite(output_5, (e131.data[4] > 127) ? HIGH : LOW);
digitalWrite(output_6, (e131.data[5] > 127) ? HIGH : LOW);
digitalWrite(output_7, (e131.data[6] > 127) ? HIGH : LOW);
digitalWrite(output_8, (e131.data[7] > 127) ? HIGH : LOW);
}//end we have data

} // end void loop