## Changelog

**Usage note:** please add new changelog information here at the beginning

##### 2020-04-12
* merged github pull requests #22 and #23
*  #22 delphin0850 committed on 25 Mar 2019: HTML Fix to save the parameters
*  #23 mvn23 committed on 13 Apr 2019: Fix serial generating and parsing
* update library "pubsubclient" to v2.7

##### 2018-08-09
* now it's possible to use the dongle only via the WebUI, by configure NO MQTT broker 
* a heartbeat function lets blink the NodeMCU/ESP's LED every 4sec

##### 2018-07-07 v0.7
* rework use of web_log_message[] array, now a circular buffer
* fix logging of "incoming MQTT command unknown"

##### 2018-07-01
* merge https://github.com/madmartin/Jarolift_MQTT/pull/5/ (partially)
* adds SNTP time fetch and timestamps in log

##### 2018-06-24 v0.6
* make debug output on serial console configurable
* fix Firefox issue in edit channel names (thanks to sidddy)
* add mqtt "sendconfig" command: "cmd/jarolift/sendconfig". the dongle replies with json encoded config data
* check and confirm running with ESP8266 core 2.4.1
* WebUI: enhance layout of system menu, display version
* system menu: serial number prefix is displayed and configurable in hexadecimal now
* system menu: new field: device counter now configurable

##### 2018-06-09 v0.6-rc2
* add logging of WiFi connect and disconnect events. WiFi reconnect works well when compiled with ESP8266 core 2.3.0

##### 2018-06-05 v0.6-rc2
* send device counter to MQTT server after each command
* add subdirectory /doc for misc documentation, beginning with file describing EEPROM memory layout
* implement new connect logic for MQTT, which makes an effective reconnect
* add "uptime in seconds" timestamp to log

##### 2018-06-01 v0.6-rc1
* versionize config data in EEPROM
* introduce configurable mqtt devicetopic
* fix LWT message handling when devicetopic is reconfigured
* Web-UI:
  * explain MQTT better
  * add example for Home Assistant
  * on shutter page: add channelnumber to each channel
* v0.6-rc1
* program version is logged on startup

##### 2018-05-24
* add LWT to mqtt connect
* mqtt client-id must be unique in the network, so add code to append ESP-ID to default
  client-id to make it unique

##### 2018-05-18
* Admin-Mode changed to Access-Point only
* add double reset detector to re-enter Admin-Mode after initial setUpClass
* fixed lots of typos
* fixed inconsistent commenting style
* fixes wording in config webserver

##### 2018-05-15 v0.5
* changed length of SSID to 32 and WPA passwort to 64. needs reconfiguration of MQTT port number
* insert GPLv3 licensing information into all source files

##### 2017-11-11 v0.4a
* improved senden-routine to prevent wdt to reset the device and get more compatibility (controlling and learing).

##### 2017-11-02
* added a switch (bool regular_learn = true; )to choose which learn method to use.
* Regular method is Up+Down at the same time followed by a stop.

##### 2017-10-30
* fixed an error which prevented to learn a motor to the dongle
* reconfigured wifi as STA.

##### 2017-08-30 v0.4
* Added ESP-Reset via fhem. Added shade command to set the shutter to a predefined position.
* Position is set via the "ssp(channel)" cmd.
* Added Group Reception.

##### 2017-08-19 v0.3
* Added WiFi reconnect routine. (THX to Markus)

##### 2017-07-28 v0.2
* Modified rx/tx switch for not getting stuck (line 1182/1192)

##### 2017-07-20 v0.1
* added reception of jarolift remotes and fhem status update

