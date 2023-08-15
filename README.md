[![PlatformIO CI](https://github.com/pfichtner/SipDoorbell/actions/workflows/platform-io-ci.yml/badge.svg)](https://github.com/pfichtner/SipDoorbell/actions/workflows/platform-io-ci.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

SipDoorBell
-----------

ESP8266-based doorbell that does initiate SIP calls. 

Why not...?
-----------
- Wifi Türklingel, ESP-Überallklingel and others: 
  - no 433 MHz support
  - projects rely on TR-064
  - hard coded values (WiFi settings, SIP settings, ...) (on the other side: This is a security feature)

- ESPHome
  - 433 MHz support very limited
  - cannot configure variables that should be on the node (at least not via an included HTML UI)
  - couldn't find a way to easily pass parameters (SIP-User, SIP-Password, ...) to an ESPHome custom component
  - Perhaps the last two items are wrong, see
    - https://esphome.io/web-api/index.html#number (configure values stored on the node)
    - https://gist.github.com/liads/c702fd4b8529991af9cd52d03b694814 (configuring an ESPHome custom component)

Features
--------
- Configurable via Web-Interface so no need to compile your own firmware, you just need to download and to flash the microcontroller (see https://tasmota.github.io/docs/Getting-Started/#flashing for example for how to flash a D1 mini). Please note: Those values can be changed by anyone having access to your network since there is no authentication/authorization (yet) implemented! 
- Acts as normal SIP-Client so it can be used not only with TR064-enabled devices but every SIP-Server/SIP-enabled hardware
- Can can be initiated via 433 MHz signal (e.g. wireless doorbell sender)
- Can can be initiated via button (e.g. a simple push button and/or optocoppler to make an existent doorbell a smart doorbell). The call gets initiated when the pin gets HIGH, so when connecting a button you would have to use pulldown resistor
- Can switch GPIO pin on ring (e.g. to visualize via LED and/or do some action on another microcontroller)
- Can publish mqtt messages on ring so other tasks can be done via integration platforms like ioBroker, home assistant, node red, ...

TODOs
-----
- protect HTTP access
- MQTT status updates (online/offline) via LWT
- Listen for offhooking and do not hangup by ourself then (otherwise the call gets killed and the door could possible not being opened)
- Learn mode for 433 MHz signals? (could be realized via hw-button as well via sw-button on the dashboard)
- Support for door relais, see https://www.mikrocontroller.net/topic/444994#5814241 https://de.wikipedia.org/wiki/Mehrfrequenzwahlverfahren#%C3%9Cbertragungsmodi_bei_Internettelefonie
- Could include TR-064 as well as alternative to SIP

Projects used
-------------
- ESP8266 IoT Framework
- knolleary/PubSubClient
- https://github.com/1technophile/rc-switch.git
- https://github.com/dl9sec/ArduinoSIP.git (fork https://github.com/pfichtner/ArduinoSIP)

Possible useful projects
------------------------
- https://github.com/RetepRelleum/SipMachine
- https://github.com/rousir/ArduinoVoip
- https://github.com/chrta/sip_call

