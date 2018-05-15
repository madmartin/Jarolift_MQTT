# Jarolift_MQTT

Controlling Jarolift(TM) TDEF 433MHz radio shutters via ESP8266 and CC1101 Transceiver Module in asynchronous mode.
Experimental version.
Use at your own risk. For private/educational use only. (Keeloq algorithm licensed only to TI Microcontrollers)

This is the MQTT version of Jarolift_RX_TX_FHEM, which was originally developed to operate with the 
FHEM home automation server. You find the FHEM version on the project's homepage.


### Author

Written by Steffen Hille in Nov, 2017


### Project Homepage and Forum

The project home is here [Project Home](http://www.bastelbudenbuben.de/2017/04/25/protokollanalyse-von-jarolift-tdef-motoren/)
If you need support please use the forum [Project Forum](http://www.bastelbudenbuben.de/forum/)


### License
The main code of Jarolift_MQTT is licensed under the GPLv3. 
The provided libraries have different licenses, look into the respective files for more information.
* CC1101 library        LGPL-License
* Keeloq library        unknown License
* PubSubClient-2.6.09   "AS-IS"-License

### Necessary Hardware:

* a NodeMCU board or similar, based on ESP8266 microcontroller
* a CC1101 Transmitter board
* some connection cables

Photos and hints on project home website

 
### Prepare Arduino IDE and libraries

Download an install the Arduino IDE from https://www.arduino.cc/en/Main/Software
(as time of writing the current version is 1.8.5)

Locate your arduino sketch directory - this is $HOME/Arduino/ in linux
Download and / or unpack this project folder to a subfolder of this sketch directory

Locate the user library folder of your arduino sketch directory
Default under linux: $HOME/Arduino/lib/

copy the folder PubSubClient-2.6.09 to the arduino library folder
copy the folder KeeloqLib to the arduino library folder


### Compile

Open the Arduino IDE and open the project .ino file (Jarolift_MQTT.ino)
FIXME write more detailed description


Check the board settings 
In menu "Tools" set the correct board type. If you have any kind of NodeMCU, the board
``NodeMCU 1.0 (ESP-12E Module`` will probably work.
Dont forget to set the correct serial port (Tools->Port) every time after plugging the USB connector of the NodeMCU into the computer.



Uploading files to SPIFFS (ESP8266 internal filesystem)
------------------------------

*ESP8266FS* is a tool which integrates into the Arduino IDE. It adds a
menu item to *Tools* menu for uploading the contents of sketch data
directory into ESP8266 flash file system.

-  Download the [SPIFFS tool](https://github.com/esp8266/arduino-esp8266fs-plugin/releases/download/0.3.0/ESP8266FS-0.3.0.zip)
-  In your Arduino sketchbook directory, create ``tools`` directory if
   it doesn't exist yet
-  Unpack the tool into ``tools`` directory (the path will look like
   ``<home_dir>/Arduino/tools/ESP8266FS/tool/esp8266fs.jar``)
-  Restart Arduino IDE
-  Open the Jarolift_MQTT sketch
-  Make sure you have selected a board, port, and closed Serial Monitor
-  Select Tools > ESP8266 Sketch Data Upload. This should start
   uploading the files into ESP8266 flash file system. When done, IDE
   status bar will display ``SPIFFS Image Uploaded`` message.


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


### Usage instructions

The running Jarolift Dongle does some debug output on the serial console.
Console Speed is 115200 Bit/s

There is a menu to configure some settings.
Press "m" to enter menu.




### Contribute

You can contribute to Jarolift_MQTT by
- providing Pull Requests (Features, Proof of Concepts, Language files or Fixes)
- testing new released features and report issues
- please try to follow the [Style-Guide](https://www.gnu.org/prep/standards/html_node/Writing-C.html#Writing-C)
