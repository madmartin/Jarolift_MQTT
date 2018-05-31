/*  Controlling Jarolift TDEF 433MHZ radio shutters via ESP8266 and CC1101 Transceiver Module in asynchronous mode.
    Copyright (C) 2017-2018 Steffen Hille et al.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef GLOBAL_H
#define GLOBAL_H

#define ACCESS_POINT_NAME      "Jarolift-Dongle"  // default SSID for Admin-Mode
#define ACCESS_POINT_PASSWORD  "12345678"         // default WLAN password for Admin-Mode
#define AdminTimeOut           180                // Defines the time in seconds, when the Admin-Mode will be disabled

ESP8266WebServer server(80);                      // The Webserver

boolean AdminEnabled = false;                     // Admin-Mode opens AccessPoint for configuration
int AdminTimeOutCounter = 0;                      // Counter for Disabling the Admin-Mode
int led_pin = 2;                                  // GPIO Pin number for LED
Ticker tkSecond;                                  // Second - Timer for Updating Datetime Structure

String web_cmd = "";                              // trigger to run a command whenever a action button has been pressed on the web interface
int web_cmd_channel;                              // keeps the respective channel ID for the web_cmd

String web_log = "";                              // used to store log information for displaying in webIF
String web_log_message[40];
int web_log_message_count = 0;


struct strConfig {
  uint16_t cfgVersion;
  String  ssid;
  String  password;
  byte    ip[4];
  byte    netmask[4];
  byte    gateway[4];
  boolean dhcp;
  byte    mqtt_broker_addr[4];
  String  mqtt_broker_port;
  String  mqtt_broker_client_id;
  String  mqtt_broker_username;
  String  mqtt_broker_password;
  String  mqtt_devicetopic;
  String  master_msb;
  String  master_lsb;
  boolean learn_mode;         // If set to true, regular learn method is used (up+down, followed by stop).
                              // If set to false another method for older versions of Jarolift motors is used.
  String  serial;
  String  name_c0;
  String  name_c1;
  String  name_c2;
  String  name_c3;
  String  name_c4;
  String  name_c5;
  String  name_c6;
  String  name_c7;
  String  name_c8;
  String  name_c9;
  String  name_c10;
  String  name_c11;
  String  name_c12;
  String  name_c13;
  String  name_c14;
  String  name_c15;

} config;


//####################################################################
// Function to write Log to both, serial and Weblog
//####################################################################
void WriteLog(String msg, boolean new_line = false)
{
  if (web_log_message_count == 40) {
    for ( int i = 1; i < 40;  ++i ) {
      web_log_message[i - 1] = web_log_message[i];
    }
    web_log_message[39] = "";
    web_log_message[40] = "";
    web_log_message_count = 39;
  }

  if (new_line == true) {
    web_log_message[web_log_message_count] = web_log_message[web_log_message_count] + " " + msg;
    web_log_message_count++;
    //web_log = web_log + " " + msg + "\n";
    Serial.println(" " + msg);
  } else {
    web_log_message[web_log_message_count] = web_log_message[web_log_message_count] + " " + msg;
    //web_log = web_log + " " + msg;
    Serial.print(" " + msg);
  }

}

//####################################################################
// Function to connect to Wifi
//####################################################################
void ConfigureWifi()
{
  WriteLog("[INFO] - connecting to", false);
  WriteLog(config.ssid, false);
  WiFi.mode(WIFI_STA);
  WiFi.begin (config.ssid.c_str(), config.password.c_str());
  if (!config.dhcp)
  {
    WiFi.config(IPAddress(config.ip[0], config.ip[1], config.ip[2], config.ip[3] ),  IPAddress(config.gateway[0], config.gateway[1], config.gateway[2], config.gateway[3] ) , IPAddress(config.netmask[0], config.netmask[1], config.netmask[2], config.netmask[3] ));
  }

  int timeout_count  = 0;
  while (WiFi.status () != WL_CONNECTED) {
    if (timeout_count > 30)
    {
      break;
    }
    timeout_count ++;
    delay(500);
    WriteLog(".", false);
  }
  if (timeout_count > 10)
  {
    WriteLog("failed!", true);
  } else {
    WriteLog("connected!", true);
    WriteLog("[INFO] - IP address:", false);
    WriteLog(WiFi.localIP().toString(), true);
  }
}

//################################################################
// Function to write newest version of configuration into EEPROM
//################################################################
void WriteConfig()
{
  WriteLog("[INFO] - write config to EEPROM", true);
  EEPROM.write(120, 'C');
  EEPROM.write(121, 'f');
  EEPROM.write(122, 'g');
  EEPROM.put(123, config.cfgVersion);

  EEPROM.write(128, config.dhcp);

  EEPROM.write(144, config.ip[0]);
  EEPROM.write(145, config.ip[1]);
  EEPROM.write(146, config.ip[2]);
  EEPROM.write(147, config.ip[3]);

  EEPROM.write(148, config.netmask[0]);
  EEPROM.write(149, config.netmask[1]);
  EEPROM.write(150, config.netmask[2]);
  EEPROM.write(151, config.netmask[3]);

  EEPROM.write(152, config.gateway[0]);
  EEPROM.write(153, config.gateway[1]);
  EEPROM.write(154, config.gateway[2]);
  EEPROM.write(155, config.gateway[3]);

  EEPROM.write(156, config.mqtt_broker_addr[0]);
  EEPROM.write(157, config.mqtt_broker_addr[1]);
  EEPROM.write(158, config.mqtt_broker_addr[2]);
  EEPROM.write(159, config.mqtt_broker_addr[3]);

  WriteStringToEEPROM(164, config.ssid);
  WriteStringToEEPROM(196, config.password);

  WriteStringToEEPROM(262, config.mqtt_broker_port);
  WriteStringToEEPROM(270, config.master_msb);
  WriteStringToEEPROM(330, config.master_lsb);
  EEPROM.write(350, config.learn_mode);
  WriteStringToEEPROM(366, config.serial);

  WriteStringToEEPROM(375, config.mqtt_broker_client_id);
  WriteStringToEEPROM(400, config.mqtt_broker_username);
  WriteStringToEEPROM(450, config.mqtt_broker_password);
  WriteStringToEEPROM(1300, config.mqtt_devicetopic);

  WriteStringToEEPROM(500, config.name_c0);
  WriteStringToEEPROM(550, config.name_c1);
  WriteStringToEEPROM(600, config.name_c2);
  WriteStringToEEPROM(650, config.name_c3);
  WriteStringToEEPROM(700, config.name_c4);
  WriteStringToEEPROM(750, config.name_c5);
  WriteStringToEEPROM(800, config.name_c6);
  WriteStringToEEPROM(850, config.name_c7);
  WriteStringToEEPROM(900, config.name_c8);
  WriteStringToEEPROM(950, config.name_c9);
  WriteStringToEEPROM(1000, config.name_c10);
  WriteStringToEEPROM(1050, config.name_c11);
  WriteStringToEEPROM(1100, config.name_c12);
  WriteStringToEEPROM(1150, config.name_c13);
  WriteStringToEEPROM(1200, config.name_c14);
  WriteStringToEEPROM(1250, config.name_c15);

  EEPROM.commit();
  delay(1000);
}

//####################################################################
// Function to read configuration from EEPROM
//####################################################################
boolean ReadConfig()
{
  WriteLog("[INFO] - read config from EEPROM . . .", false);
  config.cfgVersion = 0;

  if (EEPROM.read(120) == 'C' && EEPROM.read(121) == 'F'  && EEPROM.read(122) == 'G' )
  {
    WriteLog("config version 1 found", true);
    config.cfgVersion = 1;
  } else
  {
    if (EEPROM.read(120) == 'C' && EEPROM.read(121) == 'f'  && EEPROM.read(122) == 'g' )
    {
      EEPROM.get(123,config.cfgVersion);
      WriteLog("config version "+ (String) config.cfgVersion+ " found", true);
    }
  }
  if (config.cfgVersion==0)
  {
    WriteLog(" no configuration found in EEPROM!!!!", true);
    return false;
  }
  if (config.cfgVersion<=2)   // read config parts up to version 2
  {
    config.dhcp = EEPROM.read(128);
    config.ip[0] = EEPROM.read(144);
    config.ip[1] = EEPROM.read(145);
    config.ip[2] = EEPROM.read(146);
    config.ip[3] = EEPROM.read(147);
    config.netmask[0] = EEPROM.read(148);
    config.netmask[1] = EEPROM.read(149);
    config.netmask[2] = EEPROM.read(150);
    config.netmask[3] = EEPROM.read(151);
    config.gateway[0] = EEPROM.read(152);
    config.gateway[1] = EEPROM.read(153);
    config.gateway[2] = EEPROM.read(154);
    config.gateway[3] = EEPROM.read(155);
    config.mqtt_broker_addr[0] = EEPROM.read(156);
    config.mqtt_broker_addr[1] = EEPROM.read(157);
    config.mqtt_broker_addr[2] = EEPROM.read(158);
    config.mqtt_broker_addr[3] = EEPROM.read(159);

    config.ssid = ReadStringFromEEPROM(164,32);
    config.password = ReadStringFromEEPROM(196,64);
    config.mqtt_broker_port = ReadStringFromEEPROM(262,6);
    config.master_msb = ReadStringFromEEPROM(270,10);
    config.master_lsb = ReadStringFromEEPROM(330,10);
    config.learn_mode = EEPROM.read(350);
    config.serial = ReadStringFromEEPROM(366,10);

    config.mqtt_broker_client_id = ReadStringFromEEPROM(375,25);
    config.mqtt_broker_username = ReadStringFromEEPROM(400,25);
    config.mqtt_broker_password = ReadStringFromEEPROM(450,25);

    config.name_c0 = ReadStringFromEEPROM(500,25);
    config.name_c1 = ReadStringFromEEPROM(550,25);
    config.name_c2 = ReadStringFromEEPROM(600,25);
    config.name_c3 = ReadStringFromEEPROM(650,25);
    config.name_c4 = ReadStringFromEEPROM(700,25);
    config.name_c5 = ReadStringFromEEPROM(750,25);
    config.name_c6 = ReadStringFromEEPROM(800,25);
    config.name_c7 = ReadStringFromEEPROM(850,25);
    config.name_c8 = ReadStringFromEEPROM(900,25);
    config.name_c9 = ReadStringFromEEPROM(950,25);
    config.name_c10 = ReadStringFromEEPROM(1000,25);
    config.name_c11 = ReadStringFromEEPROM(1050,25);
    config.name_c12 = ReadStringFromEEPROM(1100,25);
    config.name_c13 = ReadStringFromEEPROM(1150,25);
    config.name_c14 = ReadStringFromEEPROM(1200,25);
    config.name_c15 = ReadStringFromEEPROM(1250,25);
  }
  if (config.cfgVersion==2)
  {                                       // read config parts of version 2
    config.mqtt_devicetopic = ReadStringFromEEPROM(1300,20);
  } else
  {                                       // upgrade config to version 2
    config.mqtt_devicetopic = "jarolift"; // default devicetopic
    config.cfgVersion = 2;
    // clear EEPROM space
    int index = 0;
    for (int index = 120 ; index < 4096 ; index++) {
      EEPROM.write(index, 0);
    }
    WriteLog("[INFO] - config update to version 2 - mqtt_devicetopic set to default", true);
    WriteConfig();
  }
  return true;
}

//############################################################################
// Function to initialize configuration, either defaults or read from EEPROM
//############################################################################
void InitializeConfigData()
{
  char chipIdString[10];
  sprintf(chipIdString,"-%08x",ESP.getChipId());

  // apply default config if saved configuration not yet exist
  if (!ReadConfig())
  {
    // DEFAULT CONFIG
    config.cfgVersion = 2;
    config.ssid = "MYSSID";
    config.password = "MYPASSWORD";
    config.dhcp = true;
    config.ip[0] = 192; config.ip[1] = 168; config.ip[2] = 1; config.ip[3] = 100;
    config.netmask[0] = 255; config.netmask[1] = 255; config.netmask[2] = 255; config.netmask[3] = 0;
    config.gateway[0] = 192; config.gateway[1] = 168; config.gateway[2] = 1; config.gateway[3] = 1;
    config.mqtt_broker_addr[0] = 192; config.mqtt_broker_addr[1] = 168; config.mqtt_broker_addr[2] = 1; config.mqtt_broker_addr[3] = 1;
    config.mqtt_broker_port = "1883";
    config.mqtt_broker_client_id = "JaroliftDongle";
    config.mqtt_broker_client_id += chipIdString;
    config.mqtt_devicetopic = "jarolift";
    config.master_msb = "0x12345678";
    config.master_lsb = "0x12345678";
    config.learn_mode = true;
    config.serial = "12345600";
    WriteConfig();
    WriteLog("[INFO] - default config applied", true);
  }

  // check if mqtt client-ID needs migration to new unique ID
  if (config.mqtt_broker_client_id == "JaroliftDongle") {
    config.mqtt_broker_client_id += chipIdString;
    WriteLog("[INFO] - mqtt_broker_client_id changed to "+ config.mqtt_broker_client_id, true);
    WriteConfig();
  }
}


void Admin_Mode_Timeout()
{
  AdminTimeOutCounter++;
}
#endif
