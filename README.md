SipDoorBell
-----------

ESP8266-based doorbell that does initiate SIP calls. 

Why not...?
-----------
- Wifi Türklingel, ESP-Überallklingel and others: 
  - no 433 MHz support
  - projects rely on TR-064
  - hard coded values (WiFi settings, SIP settings, ...) (on the other side: This is a security feature)

- ESPHome -> 433 MHz support very limited
- ESPHome -> cannot configure variables that should be on the node

Features
--------
- Configurable via Web-Interface so no need to compile your own firmware, you just need to download and to flash the microcontroller (see https://tasmota.github.io/docs/devices/Wemos-D1-Mini/ for example for how to flash a D1 mini). Please note: Those values can be changed by anyone having access to your network since there is no authentication/authorization (yet) implemented! 
- Acts as normal SIP-Client so it can be used not only with TR064-enabled devices but every SIP-Server/SIP-enabled hardware
- Can can be initiated via 433 MHz signal (e.g. wireless doorbell sender)
- Can can be initiated via button (e.g. a simple push button and/or optocoppler to make an existent doorbell a smart doorbell)
- Can switch GPIO pin on ring (e.g. to visualize via LED and/or do some action on another microcontroller)
- Can publish mqtt messages on ring so other tasks can be done via integration platforms like ioBroker, home assistant, node red, ...

TODOs
-----
- artefact releasing via GHA action
- WLAN-AP with PSK including "+" not working -> https://github.com/maakbaas/esp8266-iot-framework/issues/155
- protect HTTP access
- Listen for offhocking and do not hangup by ourself then (otherwise the call gets killed and the door could possible not being opened)
- Learn mode for 433 MHz signals?
- Support for door relais, see https://www.mikrocontroller.net/topic/444994#5814241 https://de.wikipedia.org/wiki/Mehrfrequenzwahlverfahren#%C3%9Cbertragungsmodi_bei_Internettelefonie
- Could include TR-064 as well as alternative to SIP

Projects used
-------------
- ESP8266 IoT Framework
- knolleary/PubSubClient
- https://github.com/1technophile/rc-switch.git
- https://github.com/dl9sec/ArduinoSIP.git

Possible useful projects
------------------------
- https://github.com/RetepRelleum/SipMachine
- https://github.com/rousir/ArduinoVoip
- https://github.com/chrta/sip_call

