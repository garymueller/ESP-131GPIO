# ESP-131GPIO

This is arduino firmware that will listen to a E1.31 (sACN) data soure and will drive a boards GPIO pins. The firmware supports ESP8266 and is currently configured to support the WEMOS mini.  This was designed to drive relays for static christmas display lights being driven by Vixen, XLights or other application that outouts a E1.31 data stream. The board supports the following ...

* Status page
* Configuration Page
  * Network
  * E131
* Test Page

* REST API - to set relay states through an external interface
  * /SetRelay?relay=[0-#]&checked=[true|false]
* Captive Portal (Optional) - Automatically start the arduino in access point mode if the WiFi settings aren't able to connect.

# Build Environments

[Arduino IDE](https://www.arduino.cc/en/main/software)

[Arduino 8266](https://github.com/esp8266/Arduino)

# Libraries
[Arduino WiFi](https://www.arduino.cc/en/Reference/WiFi)

[Arduino JSON](https://arduinojson.org/)

[Arduino 1.31](https://github.com/forkineye/E131)

# Software / Library Installation

Download and install the above build environments and libraries.  The build environments can be installed by downloading and installing the appropriate software.  The libraries can be installed through the ide with manage libraries.

# Installing
1. I initially installed the windows CH340 drivers so that it would recognize the arduino chipset. 
1. The Wemos requires you to jump pins D3 and GND to start the board into programming mode.  
1. Plug the arduino into your computers USB. 
1. Click the upload button to compile and transfer the firmware to the arduino. 
1. Then select the "ESP8266 sketch data upload" found under the tools menu. This will upload the html to the arduino.
