Why?
----
Wifi Türklingel, ESP-Überallklingel and others: 
- no 433 MHz support
- projects rely on TR-064
- hard coded values (WiFi settings, SIP settings, ...)

ESPHome -> 433 MHz support very limited
ESPHome -> cannot configure variables that should be on the node

Features
--------
- Acts as normal SIP-Client so it can be used not only with TR064-enabled devices but every SIP-Server/SIP-enabled hardware
- Can can be initiated via 433 MHz signal (e.g. wireless doorbell sender)
- Can can be initiated via button (e.g. a simple push button and/or optocoppler to make an existent doorbell a smart doorbell)
- Can switch GPIO pin on ring (e.g. to visualize via LED and/or do some action on another microcontroller)
- Can publish mqtt messages on ring so other tasks can be done via integration platforms like ioBroker, home assistant, node red, ...


TODOs
-----
WLAN-AP with PSK including "+" not working -> File issue
protect HTTP access
Learn mode for 433 MHz signals?
