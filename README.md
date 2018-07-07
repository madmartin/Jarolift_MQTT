# Jarolift_MQTT

Controlling Jarolift(TM) TDEF 433MHz radio shutters via ESP8266 and CC1101 Transceiver Module in asynchronous mode.
Experimental version.
Use at your own risk. For private/educational use only. (Keeloq algorithm licensed only to TI Microcontrollers)
This project is not affiliated in any way with the vendor of the Jarolift components.
Jarolift is a Trademark of Schöneberger Rolladenfabrik GmbH & Co. KG

This is the MQTT version of Jarolift_RX_TX_FHEM, which was originally developed to operate with the 
FHEM home automation server. You find the original FHEM version on the project's homepage.


### Author

Written by Steffen Hille in Nov, 2017


### Project Homepage and Forum

The project home is here: [Project Home](http://www.bastelbudenbuben.de/2017/04/25/protokollanalyse-von-jarolift-tdef-motoren/)
If you need support please use the forum [Project Forum](http://www.bastelbudenbuben.de/forum/)


### License
The main code of Jarolift_MQTT is licensed under the GPLv3. 
The provided libraries have different licenses, look into the respective files for more information.
* CC1101 library        LGPL-License
* DoubleResetDetector   MIT License
* Keeloq library        unknown License
* PubSubClient-2.6.09   "AS-IS"-License
* simpleDSTadjust       unknown License

### Necessary Hardware:

* a NodeMCU board or similar, based on ESP8266 microcontroller
* a CC1101 Transmitter board
* some connection cables

Photos and hints on project home website

 
### Prepare Arduino IDE and libraries

Download an install the Arduino IDE from [Arduino website](//www.arduino.cc/en/Main/Software)
(as time of writing the current version is 1.8.5)

Locate your arduino sketch directory - this is `$HOME/Arduino/` in linux

Download and / or unpack this project folder to a subfolder of this sketch directory

Locate the user library folder of your arduino sketch directory

Default under linux: `$HOME/Arduino/libraries/`

* copy the folder `DoubleResetDetector` to the arduino library folder
* copy the folder `PubSubClient-2.6.09` to the arduino library folder
* copy the folder `KeeloqLib` to the arduino library folder
* copy the folder `simpleDSTadjust` to the arduino library folder


### Compile

Open the Arduino IDE and open the project .ino file (`Jarolift_MQTT.ino`)
TODO write a more detailed description

#### Check the board settings
In menu "Tools" set the correct board type. If you have any kind of NodeMCU, the board
``NodeMCU 1.0 (ESP-12E Module`` will probably work.
The following settings have proofed to work with a NodeMCU v1.0/V2 [comparison of boards](https://frightanic.com/iot/comparison-of-esp8266-nodemcu-development-boards/)

##### ESP Core 2.3.0
* Board: "NodeMCU 1.0 (ESP-12E Module)"
* Flash Size: "4M (1M SPIFFS)"
* lwIP variant: "v2 Prebuilt (MSS)536)"
* CPU Frequency: "80 MHz"
* Upload speed: "115200"

##### ESP Core 2.4.1
* Board: "NodeMCU 1.0 (ESP-12E Module)"
* Flash Size: "4M (1M SPIFFS)"
* lwIP variant: "v2 Lower Memory"
* CPU Frequency: "80 MHz"
* Upload speed: "115200"


Dont forget to set the correct serial port (Tools->Port) after plugging the USB connector of the NodeMCU into the computer.

Upload the compiled sketch into the NodeMCU. Second, you need to upload the files from the data directory into the SPIFFS area. This is only necessary on first use of the NodeMCU and then only after the content of the files has changed.


### Uploading files to SPIFFS (ESP8266 internal filesystem)

*ESP8266FS* is a tool which integrates into the Arduino IDE. It adds a
menu item to *Tools* menu for uploading the contents of sketch data
directory into ESP8266 flash file system.

-  Download the [SPIFFS tool](https://github.com/esp8266/arduino-esp8266fs-plugin/releases/download/0.3.0/ESP8266FS-0.3.0.zip)
-  In your Arduino sketchbook directory, create `tools` directory if
   it doesn't exist yet
-  Unpack the tool into `tools` directory (the path will look like
   `$HOME/Arduino/tools/ESP8266FS/tool/esp8266fs.jar`)
-  Restart Arduino IDE
-  Open the Jarolift_MQTT sketch
-  Make sure you have selected a board, port, and closed Serial Monitor
-  Select Tools > ESP8266 Sketch Data Upload. This should start
   uploading the files into ESP8266 flash file system. When done, IDE
   status bar will display `SPIFFS Image Uploaded` message.


### Cabling instructions
ESP | CC1101 | Remark
------|------|------
D8 | CSN
D2 | GDO0 | (perhaps 1Kohm to GND)
D1 | GDO2
D6 | SO(GDO1)
D5 | SCLK
D7 | SI
GND | GND
VCC | VCC | 3.3 Volt !!!


### Setup instructions

The configuration of the Jarolift Dongle is stored in the EEPROM memory of the ESP8266. On first initialisation, when no configuration is found, it is initialized with some default values and the Dongle turns on the Admin-Mode.

In Admin-Mode, the blue LED on the ESP submodule is turned on and the Dongle creates an WLAN-Access-Point with the SSID `Jarolift-Dongle`, protectet with the WPA-Passwort `12345678`. Now you have 180 seconds (3 minutes) time to connect to the WLAN Accesspoint and visit the configuration webserver on

http://192.168.4.1

Admin-Mode quits after the 180 second timeout or when you restart the Dongle from the configuration webserver.

If you need the Admin-Mode later, just press the "Reset" button on the NodeMCU module two times within 10 seconds. This double-reset will be detected and the Dongle enters Admin-Mode again, showing this with the blue LED turned on.

The running Jarolift Dongle does some debug output on the serial console.
Console Speed is 115200 Bit/s


### Known issues

* on the WebUI shutter page, while trying to edit, the "finish" button has no effect. This is a known problem with Firefox browser, and is known to work with Chrome/Chromium.
* after upgrade to this release, due to changes of the EEPROM memory layout the MQTT port parameter needs to be set again.
* when Dongle looses the WLAN connection, the reconnect behaviour may not be good. There is a known problem with failing WLAN connection after a ESP restart with ESP Core 2.4.0, so better compile this project with ESP Core 2.3.0. or ESP Core 2.4.1. See what other projects say about this: [#1](https://github.com/tzapu/WiFiManager/issues/532) [#2](https://github.com/arendst/Sonoff-Tasmota/wiki/What%27s-New#esp8266arduino-core-information)
* after flashing the NodeMCU board, on the first run of the fresh flashed sketch, any kind of restart (through WebUI or after a crash) of the ESP8266 may not properly work, causes the NodeMCU board board to hang. On the serial monitor, you see output like
```
handleFileRead: /favicon.png
 [INFO] - Writing Config to EEPROM

 ets Jan  8 2013,rst cause:2, boot mode:(1,6)


 ets Jan  8 2013,rst cause:4, boot mode:(1,6)

wdt reset
```
* this is a hardware issue, solution: power cycle your NodeMCU after flashing, pressing the reset button is not sufficient

* sometimes opening the "system" page in the WebUI causes a crash (Exception 28) of the ESP8266. The cause is not found yet.

### Contribute

You can contribute to Jarolift_MQTT by
- providing Pull Requests (Features, Proof of Concepts, Language files or Fixes)
- testing new released features and report issues
- please try to follow the [Style-Guide](https://www.gnu.org/prep/standards/html_node/Writing-C.html#Writing-C)
  [another Style-Guide](https://google.github.io/styleguide/cppguide.html)
