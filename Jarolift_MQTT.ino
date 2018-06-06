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


// Changelog: see CHANGES.md

/*
  Kanal  S/N           DiscGroup_8-16             DiscGroup_1-8     SN(last two digits)
  0       0            0000 0000                   0000 0001           0000 0000
  1       1            0000 0000                   0000 0010           0000 0001
  2       2            0000 0000                   0000 0100           0000 0010
  3       3            0000 0000                   0000 1000           0000 0011
  4       4            0000 0000                   0001 0000           0000 0100
  5       5            0000 0000                   0010 0000           0000 0101
  6       6            0000 0000                   0100 0000           0000 0110
  7       7            0000 0000                   1000 0000           0000 0111
  8       8            0000 0001                   0000 0000           0000 0111
  9       9            0000 0010                   0000 0000           0000 0111
  10      10           0000 0100                   0000 0000           0000 0111
  11      11           0000 1000                   0000 0000           0000 0111
  12      12           0001 0000                   0000 0000           0000 0111
  13      13           0010 0000                   0000 0000           0000 0111
  14      14           0100 0000                   0000 0000           0000 0111
  15      15           1000 0000                   0000 0000           0000 0111
*/


#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <EEPROM.h>
#include <ESP8266WebServer.h>
#include <SPI.h>
#include <FS.h>
#include <PubSubClient.h>
#include <Ticker.h>
#include <DoubleResetDetector.h>

#include "helpers.h"
#include "global.h"
#include "html_api.h"

extern "C" {
#include <Wire.h>
#include <stdint.h>
#include "c_types.h"
#include "ets_sys.h"
#include "os_type.h"
#include "osapi.h"
#include "mem.h"
#include "user_interface.h"
#include "smartconfig.h"
#include "Arduino.h"
#include "cc1101.h"
#include <stdio.h>
#include <KeeloqLib.h>
#include <stdlib.h>
}

#define PROGRAM_VERSION "v0.6-rc2"

// Number of seconds after reset during which a
// subseqent reset will be considered a double reset.
#define DRD_TIMEOUT 10

// RTC Memory Address for the DoubleResetDetector to use
#define DRD_ADDRESS 0

// User configuration
#define Lowpulse         400    // Defines pulse-width in microseconds. Adapt for your use...
#define Highpulse        800

#define BITS_SIZE          8
byte syncWord            = 199;
int device_key_msb       = 0x0; // stores cryptkey MSB
int device_key_lsb       = 0x0; // stores cryptkey LSB
uint64_t button          = 0x0; // 1000=0x8 up, 0100=0x4 stop, 0010=0x2 down, 0001=0x1 learning
unsigned short devcnt    = 0x0; // Initial 16Bit countervalue, will be stored in EEPROM and incremented once every time a command is send
int disc                 = 0x0;
uint32_t dec             = 0;   // stores the 32Bit encrypted code
uint64_t pack            = 0;   // Contains data to send.
byte disc_low[16]        = {0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80, 0x0, 0x0, 0x0, 0x0, 0x0,  0x0,  0x0,  0x0};
byte disc_high[16]       = {0x0, 0x0, 0x0, 0x0, 0x0,  0x0,  0x0,  0x0, 0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80};
byte serials[16]         = {0x0, 0x1, 0x2, 0x3, 0x4,  0x5,  0x6,  0x7, 0x8, 0x9, 0xA, 0xB, 0xC,  0xD,  0xE,  0xF }; // Represents last serial digit in binary 1234567[8] = 0xF
byte disc_l              = 0;
byte disc_h              = 0;
byte adresses[]          = {5, 11, 17, 23, 29, 35, 41, 47, 53, 59, 65, 71, 77, 85, 91, 97 }; // Defines start addresses of channel data stored in EEPROM 4bytes s/n.
uint64_t new_serial      = 0;
int cntadr               = 110;  // Where the 16Bit counter is stored.
byte marcState;
int MqttRetryCounter = 0;                 // Counter for MQTT reconnect

// RX variables and defines
#define debounce         200              // Ignoring short pulses in reception... no clue if required and if it makes sense ;)
#define pufsize          216              // Pulsepuffer
#define DATAIN             5              // Inputport for reception
uint32_t rx_serial       = 0;
char rx_serial_array[8]  = {0};
char rx_disc_low[8]      = {0};
char rx_disc_high[8]     = {0};
uint32_t rx_hopcode      = 0;
uint16_t rx_disc_h       = 0;
byte rx_function         = 0;
int rx_device_key_msb    = 0x0;           // stores cryptkey MSB
int rx_device_key_lsb    = 0x0;           // stores cryptkey LSB
volatile uint32_t decoded         = 0x0;  // decoded hop code
volatile byte pbwrite;
volatile unsigned int lowbuf[pufsize];    // ring buffer storing LOW pulse lengths
volatile unsigned int hibuf[pufsize];     // ring buffer storing HIGH pulse lengths
volatile bool iset = false;
volatile byte value = 0;                  // Stores RSSI Value
long rx_time;
bool lcl_group = false;
char serialnr[4] = {0};
char sn[4] = {0};
int steadycnt = 0;


DoubleResetDetector drd(DRD_TIMEOUT, DRD_ADDRESS);

//####################################################################
// Receive Routine
//####################################################################
void ICACHE_RAM_ATTR measure()
{
  static long LineUp, LineDown, Timeout;
  long LowVal, HighVal;
  int pinstate = digitalRead(DATAIN); // Read current pin state
  if (micros() - Timeout > 3500) {
    pbwrite = 0;
  }
  if (pinstate)                       // pin is now HIGH, was low
  {
    LineUp = micros();                // Get actual time in LineUp
    LowVal = LineUp - LineDown;       // calculate the LOW pulse time
    if (LowVal < debounce) return;
    if ((LowVal > 300) && (LowVal < 4300))
    {
      if ((LowVal > 3650) && (LowVal < 4300)) {
        Timeout = micros();
        pbwrite = 0;
        lowbuf[pbwrite] = LowVal;
        pbwrite++;
      }
      if ((LowVal > 300) && (LowVal < 1000)) {
        lowbuf[pbwrite] = LowVal;
        pbwrite++;
        Timeout = micros();
      }
    }
  }
  else
  {
    LineDown = micros();          // line went LOW after being HIGH
    HighVal = LineDown - LineUp;  // calculate the HIGH pulse time
    if (HighVal < debounce) return;
    if ((HighVal > 300) && (HighVal < 1000))
    {
      hibuf[pbwrite] = HighVal;
    }
  }
} // void ICACHE_RAM_ATTR measure

// The connection to the hardware chip CC1101 the RF Chip
CC1101 cc1101;

//####################################################################
// sketch initialization routine
//####################################################################
void setup()
{
  EEPROM.begin(4096);
  Serial.begin(115200);
  delay(500);
  InitLog();
  WriteLog("[INFO] - starting Jarolift Dongle "+ (String)PROGRAM_VERSION, true);
  WriteLog("[INFO] - ESP-ID "+ (String)ESP.getChipId()+ " // ESP-Core  "+ ESP.getCoreVersion()+ " // SDK Version "+ ESP.getSdkVersion(), true);

  InitializeConfigData();

  // initialize the transceiver chip
  WriteLog("[INFO] - initializing the CC1101 Transceiver. If you get stuck here, it is probably not connected.", true);
  cc1101.init();
  cc1101.setSyncWord(syncWord, false);
  cc1101.setCarrierFreq(CFREQ_433);
  cc1101.disableAddressCheck();   // if not specified, will only display "packet received"

  pinMode(led_pin, OUTPUT);   // prepare LED on ESP-Chip

  // test if the WLAN SSID is on default
  // or DoubleReset detected
  if ((drd.detectDoubleReset()) || (config.ssid == "MYSSID")) {
    digitalWrite(led_pin, LOW);  // turn LED on                    // if yes then turn on LED
    AdminEnabled = true;                                           // and go to Admin-Mode
  } else {
    digitalWrite(led_pin, HIGH); // turn LED off                   // turn LED off
  }

  // enable access point mode if Admin-Mode is enabled
  if (AdminEnabled)
  {
    WriteLog("[WARN] - Admin-Mode enabled!", true);
    WriteLog("[WARN] - starting soft-AP ... ", false);
    WiFi.mode(WIFI_AP);
    WriteLog(WiFi.softAP(ACCESS_POINT_NAME, ACCESS_POINT_PASSWORD) ? "Ready" : "Failed!", true);
    //WiFi.setAutoConnect(false);
    WriteLog("[WARN] - Access Point <" + (String)ACCESS_POINT_NAME + "> activated. WPA password is "+ ACCESS_POINT_PASSWORD, true);
    WriteLog("[WARN] - you have " + (String)AdminTimeOut + " seconds time to connect and configure!", true);
    WriteLog("[WARN] - configuration webserver is http://" + WiFi.softAPIP().toString(), true);
  }
  else
  {
    // establish Wifi connection in station mode
    ConfigureWifi();
  }

  // configure webserver and start it
  server.on ( "/api", html_api );                       // command api
  SPIFFS.begin();                                       // Start the SPI flash filesystem
  server.onNotFound([]() {                              // If the client requests any URI
    if (!handleFileRead(server.uri()))                  // send it if it exists
      server.send(404, "text/plain", "404: Not Found"); // otherwise, respond with a 404 (Not Found) error
  });

  server.begin();
  WriteLog("[INFO] - HTTP server started", true);
  tkSecond.attach(1, Admin_Mode_Timeout);

  // configure MQTT client
  mqtt_client.setServer(IPAddress(config.mqtt_broker_addr[0], config.mqtt_broker_addr[1],
                                  config.mqtt_broker_addr[2], config.mqtt_broker_addr[3]),
                        config.mqtt_broker_port.toInt());
  mqtt_client.setCallback(mqtt_callback);   // define Handler for incoming messages
  mqttLastConnectAttempt = 0;

  pinMode(4, OUTPUT); // TX Pin

  // RX
  pinMode(DATAIN, INPUT_PULLUP);
  attachInterrupt(DATAIN, measure, CHANGE); // Interrupt @Inputpin
} // void setup

//####################################################################
// convert the file extension to the MIME type
//####################################################################
String getContentType(String filename) {
  if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  return "text/plain";
} // String getContentType

//####################################################################
// send the right file to the client (if it exists)
//####################################################################
bool handleFileRead(String path) {
  Serial.println("handleFileRead: " + path);
  if (path.endsWith("/")) path += "index.html";         // If a folder is requested, send the index file
  String contentType = getContentType(path);            // Get the MIME type
  if (SPIFFS.exists(path)) {                            // If the file exists
    File file = SPIFFS.open(path, "r");                 // Open it
    size_t sent = server.streamFile(file, contentType); // And send it to the client
    file.close();                                       // Then close the file again
    return true;
  }
  Serial.println("\tFile Not Found");
  return false;                                         // If the file doesn't exist, return false
} // bool handleFileRead


//####################################################################
// main loop
//####################################################################
void loop()
{

  // Call the double reset detector loop method every so often,
  // so that it can recognise when the timeout expires.
  // You can also call drd.stop() when you wish to no longer
  // consider the next reset as a double reset.
  drd.loop();

  // disable Admin-Mode after AdminTimeOut
  if (AdminEnabled)
  {
    if (AdminTimeOutCounter > AdminTimeOut)
    {
      AdminEnabled = false;
      digitalWrite(led_pin, HIGH);   // turn LED off
      WriteLog("[WARN] - Admin-Mode disabled, soft-AP terminate ...", false);
      WriteLog(WiFi.softAPdisconnect(true) ? "success" : "fail!", true);
      ConfigureWifi();
    }
  }
  server.handleClient();

  if (iset) {
    cc1101.cmdStrobe(CC1101_SCAL);
    delay(50);
    enterrx();
    iset = false;
    delay(200);
    attachInterrupt(DATAIN, measure, CHANGE); // Interrupt @Inputpin;
  }

  // Check if RX buffer is full
  if ((lowbuf[0] > 3650) && (lowbuf[0] < 4300) && (pbwrite >= 65) && (pbwrite <= 75)) {     // Decode received data...
    iset = true;
    ReadRSSI();
    pbwrite = 0;


    for (int i = 0; i <= 31; i++) {                          // extracting Hopcode
      if (lowbuf[i + 1] < hibuf[i + 1]) {
        rx_hopcode = rx_hopcode & ~(1 << i) | (0 << i);
      } else {
        rx_hopcode = rx_hopcode & ~(1 << i) | (1 << i);
      }
    }
    for (int i = 0; i <= 27; i++) {                         // extracting Serialnumber
      if (lowbuf[i + 33] < hibuf[i + 33]) {
        rx_serial = rx_serial & ~(1 << i) | (0 << i);
      } else {
        rx_serial = rx_serial & ~(1 << i) | (1 << i);
      }
    }
    rx_serial_array[0] = (rx_serial >> 24) & 0xFF;
    rx_serial_array[1] = (rx_serial >> 16) & 0xFF;
    rx_serial_array[2] = (rx_serial >> 8) & 0xFF;
    rx_serial_array[3] = rx_serial & 0xFF;

    for (int i = 0; i <= 3; i++) {                        // extracting function code
      if (lowbuf[61 + i] < hibuf[61 + i]) {
        rx_function = rx_function & ~(1 << i) | (0 << i);
      } else {
        rx_function = rx_function & ~(1 << i) | (1 << i);
      }
    }

    for (int i = 0; i <= 7; i++) {                        // extracting high disc
      if (lowbuf[65 + i] < hibuf[65 + i]) {
        rx_disc_h = rx_disc_h & ~(1 << i) | (0 << i);
      } else {
        rx_disc_h = rx_disc_h & ~(1 << i) | (1 << i);
      }
    }

    Serial.printf("serialnumber: 0x%08x // function code: 0x%02x // disc: 0x%02x\n",rx_serial,rx_function,rx_disc_h);

    rx_disc_high[0] = rx_disc_h & 0xFF;
    rx_keygen ();
    rx_decoder();
    if (rx_function == 0x4)steadycnt++;           // to detect a long press....
    else steadycnt--;
    if (steadycnt > 10 && steadycnt <= 40) {
      rx_function = 0x3;
      steadycnt = 0;
    }
    rx_disc_h = 0;
    rx_hopcode = 0;
    rx_function = 0;
  }

  // establish connection to MQTT broker
  if (WiFi.status() == WL_CONNECTED) {
    if (!mqtt_client.connected()) {
      // calculate time since last connection attempt
      long now = millis();
      // possible values of mqttLastReconnectAttempt:
      // 0  => never attempted to connect
      // >0 => at least one connect attempt was made
      if ((mqttLastConnectAttempt == 0) || (now - mqttLastConnectAttempt > MQTT_Reconnect_Interval)) {
        mqttLastConnectAttempt = now;
        // attempt to connect
        mqtt_connect();
      }
    } else {
      // client is connected, call the mqtt loop
      mqtt_client.loop();
    }
  }

  // run a CMD whenever a web_cmd event has been triggered
  if (web_cmd != "") {

    iset = true;
    detachInterrupt(DATAIN); // Interrupt @Inputpin
    delay(1);

    if (web_cmd == "up") {
      cmd_up(web_cmd_channel);
    } else if (web_cmd == "down") {
      cmd_down(web_cmd_channel);
    } else if (web_cmd == "stop") {
      cmd_stop(web_cmd_channel);
    } else if (web_cmd == "set shade") {
      cmd_set_shade_position(web_cmd_channel);
    } else if (web_cmd == "shade") {
      cmd_shade(web_cmd_channel);
    } else if (web_cmd == "learn") {
      cmd_learn(web_cmd_channel);
    } else if (web_cmd == "save and generate serials") {
      cmd_generate_serials(config.serial);
      delay(500);
      ESP.restart();
      server.send ( 200, "text/plain", "Configuration has been saved and serial numbers has been generated. System is restarting. Please refresh manually in about 30 seconds." );
    } else {
      WriteLog("[ERR ] - received unknown command from web_cmd.", true);
    }
    web_cmd = "";
  }
} // void loop

//####################################################################
// Generation of the encrypted message (Hopcode)
//####################################################################
int keeloq () {
  Keeloq k(device_key_msb, device_key_lsb);
  unsigned int result = (disc << 16) | devcnt;  // Append counter value to discrimination value
  dec = k.encrypt(result);
} // int keeloq

//####################################################################
// Keygen generates the device crypt key in relation to the masterkey and provided serial number.
// Here normal key-generation is used according to 00745a_c.PDF Appendix G.
// https://github.com/hnhkj/documents/blob/master/KEELOQ/docs/AN745/00745a_c.pdf
//####################################################################
int keygen () {

  char  charBufMSB[config.master_msb.length() + 1];
  config.master_msb.toCharArray(charBufMSB, config.master_msb.length() + 1);
  const unsigned long MasterMSB = (int)strtol(charBufMSB, NULL, 16);

  char  charBufLSB[config.master_lsb.length() + 1];
  config.master_lsb.toCharArray(charBufLSB, config.master_lsb.length() + 1);
  const unsigned long MasterLSB = (int)strtol(charBufLSB, NULL, 16);

  Keeloq k(MasterMSB, MasterLSB);
  uint64_t keylow = new_serial | 0x20000000;
  unsigned long enc = k.decrypt(keylow);
  device_key_lsb  = enc;              // Stores LSB devicekey 16Bit
  keylow = new_serial | 0x60000000;
  enc    = k.decrypt(keylow);
  device_key_msb  = enc;              // Stores MSB devicekey 16Bit

  Serial.printf("devicekey low: 0x%08x // high: 0x%08x\n",device_key_lsb, device_key_msb);
} // int keygen

//####################################################################
// Simple TX routine. Repetitions for simulate continuous button press.
// Send code two times. In case of one shutter did not "hear" the command.
//####################################################################
void senden(int repetitions) {
  pack = (button << 60) | (new_serial << 32) | dec;
  for (int a = 0; a < repetitions; a++)
  {
    digitalWrite(4, LOW);            // CC1101 in TX Mode+
    delayMicroseconds(1150);
    frame(13);                       // change 28.01.2018 default 10
    delayMicroseconds(3500);

    for (int i = 0; i < 64; i++) {

      int out = ((pack >> i) & 0x1); // Bitmask to get MSB and send it first
      if (out == 0x1)
      {
        digitalWrite(4, LOW);        // Simple encoding of bit state 1
        delayMicroseconds(Lowpulse);
        digitalWrite(4, HIGH);
        delayMicroseconds(Highpulse);
      }
      else
      {
        digitalWrite(4, LOW);        // Simple encoding of bit state 0
        delayMicroseconds(Highpulse);
        digitalWrite(4, HIGH);
        delayMicroseconds(Lowpulse);
      }
    }
    group_h();                       // Last 8Bit. For motor 8-16.

    unsigned long delaytime = micros(); // This part is necessary to prevent the wdt to reset the device.
    delay(13);
    delaytime = micros() - delaytime;
    delayMicroseconds(16000 - delaytime);
  }
} // void senden

//####################################################################
// Sending of high_group_bits 8-16
//####################################################################
void group_h() {
  for (int i = 0; i < 8; i++) {
    int out = ((disc_h >> i) & 0x1); // Bitmask to get MSB and send it first
    if (out == 0x1)
    {
      digitalWrite(4, LOW);          // Simple encoding of bit state 1
      delayMicroseconds(Lowpulse);
      digitalWrite(4, HIGH);
      delayMicroseconds(Highpulse);
    }
    else
    {
      digitalWrite(4, LOW);          // Simple encoding of bit state 0
      delayMicroseconds(Highpulse);
      digitalWrite(4, HIGH);
      delayMicroseconds(Lowpulse);
    }
  }
} // void group_h

//####################################################################
// Generates sync-pulses
//####################################################################
void frame(int l) {
  for (int i = 0; i < l; ++i) {
    digitalWrite(4, LOW);
    delayMicroseconds(400);          // change 28.01.2018 default highpulse
    digitalWrite(4, HIGH);
    delayMicroseconds(380);          // change 28.01.2018 default lowpulse
  }
} // void frame

//####################################################################
// reverses incoming groupbits
//####################################################################
unsigned int reverseBits ( unsigned int a ) {
  unsigned int rev = 0;
  int i;
  /* scans each bit of the input number */
  for ( i = 0; i < BITS_SIZE - 1; i++ )
  {
    /* checks if the bit is 1 */
    if ( a & ( 1 << i ) )
    {
      /* shifts the bit 1, starting from the MSB to LSB
         to build the reverse number
      */
      rev |= 1 << ( BITS_SIZE - 1 ) - i;
    }
  }
  return rev;
} // unsigned int reverseBits

//####################################################################
// Fast lookup-table for reversing the groupbits
//####################################################################
unsigned char reverse_byte(unsigned char x)
{
  static const unsigned char table[] = {
    0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0,
    0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0, 0x70, 0xf0,
    0x08, 0x88, 0x48, 0xc8, 0x28, 0xa8, 0x68, 0xe8,
    0x18, 0x98, 0x58, 0xd8, 0x38, 0xb8, 0x78, 0xf8,
    0x04, 0x84, 0x44, 0xc4, 0x24, 0xa4, 0x64, 0xe4,
    0x14, 0x94, 0x54, 0xd4, 0x34, 0xb4, 0x74, 0xf4,
    0x0c, 0x8c, 0x4c, 0xcc, 0x2c, 0xac, 0x6c, 0xec,
    0x1c, 0x9c, 0x5c, 0xdc, 0x3c, 0xbc, 0x7c, 0xfc,
    0x02, 0x82, 0x42, 0xc2, 0x22, 0xa2, 0x62, 0xe2,
    0x12, 0x92, 0x52, 0xd2, 0x32, 0xb2, 0x72, 0xf2,
    0x0a, 0x8a, 0x4a, 0xca, 0x2a, 0xaa, 0x6a, 0xea,
    0x1a, 0x9a, 0x5a, 0xda, 0x3a, 0xba, 0x7a, 0xfa,
    0x06, 0x86, 0x46, 0xc6, 0x26, 0xa6, 0x66, 0xe6,
    0x16, 0x96, 0x56, 0xd6, 0x36, 0xb6, 0x76, 0xf6,
    0x0e, 0x8e, 0x4e, 0xce, 0x2e, 0xae, 0x6e, 0xee,
    0x1e, 0x9e, 0x5e, 0xde, 0x3e, 0xbe, 0x7e, 0xfe,
    0x01, 0x81, 0x41, 0xc1, 0x21, 0xa1, 0x61, 0xe1,
    0x11, 0x91, 0x51, 0xd1, 0x31, 0xb1, 0x71, 0xf1,
    0x09, 0x89, 0x49, 0xc9, 0x29, 0xa9, 0x69, 0xe9,
    0x19, 0x99, 0x59, 0xd9, 0x39, 0xb9, 0x79, 0xf9,
    0x05, 0x85, 0x45, 0xc5, 0x25, 0xa5, 0x65, 0xe5,
    0x15, 0x95, 0x55, 0xd5, 0x35, 0xb5, 0x75, 0xf5,
    0x0d, 0x8d, 0x4d, 0xcd, 0x2d, 0xad, 0x6d, 0xed,
    0x1d, 0x9d, 0x5d, 0xdd, 0x3d, 0xbd, 0x7d, 0xfd,
    0x03, 0x83, 0x43, 0xc3, 0x23, 0xa3, 0x63, 0xe3,
    0x13, 0x93, 0x53, 0xd3, 0x33, 0xb3, 0x73, 0xf3,
    0x0b, 0x8b, 0x4b, 0xcb, 0x2b, 0xab, 0x6b, 0xeb,
    0x1b, 0x9b, 0x5b, 0xdb, 0x3b, 0xbb, 0x7b, 0xfb,
    0x07, 0x87, 0x47, 0xc7, 0x27, 0xa7, 0x67, 0xe7,
    0x17, 0x97, 0x57, 0xd7, 0x37, 0xb7, 0x77, 0xf7,
    0x0f, 0x8f, 0x4f, 0xcf, 0x2f, 0xaf, 0x6f, 0xef,
    0x1f, 0x9f, 0x5f, 0xdf, 0x3f, 0xbf, 0x7f, 0xff,
  };
  return table[x];
} // unsigned char reverse_byte

//####################################################################
// Calculate device code from received serial number
//####################################################################
int rx_keygen () {

  char  charBufMSB[config.master_msb.length() + 1];
  config.master_msb.toCharArray(charBufMSB, config.master_msb.length() + 1);
  const unsigned long MasterMSB = (int)strtol(charBufMSB, NULL, 16);

  char  charBufLSB[config.master_lsb.length() + 1];
  config.master_lsb.toCharArray(charBufLSB, config.master_lsb.length() + 1);
  const unsigned long MasterLSB = (int)strtol(charBufLSB, NULL, 16);

  Keeloq k(MasterMSB, MasterLSB);
  uint32_t keylow = rx_serial | 0x20000000;
  unsigned long enc = k.decrypt(keylow);
  rx_device_key_lsb  = enc;        // Stores LSB devicekey 16Bit
  keylow = rx_serial | 0x60000000;
  enc    = k.decrypt(keylow);
  rx_device_key_msb  = enc;        // Stores MSB devicekey 16Bit

  Serial.printf("devicekey low: 0x%08x // high: 0x%08x",rx_device_key_lsb, rx_device_key_msb);
} // int rx_keygen

//####################################################################
// Decoding of the hopping code
//####################################################################
int rx_decoder () {
  Keeloq k(rx_device_key_msb, rx_device_key_lsb);
  unsigned int result = rx_hopcode;
  decoded = k.decrypt(result);
  rx_disc_low[0] = (decoded >> 24) & 0xFF;
  rx_disc_low[1] = (decoded >> 16) & 0xFF;

  Serial.printf(" // decoded: 0x%08x\n\n",decoded);
} // int rx_decoder

//####################################################################
// calculate RSSI value (Received Signal Strength Indicator)
//####################################################################
void ReadRSSI()
{
  byte rssi = 0;
  rssi = (cc1101.readReg(CC1101_RSSI, CC1101_STATUS_REGISTER));
  if (rssi >= 128)
  {
    value = 255 - rssi;
    value /= 2;
    value += 74;
  }
  else
  {
    value = rssi / 2;
    value += 74;
  }
  Serial.print("CC1101_RSSI ");
  Serial.println(value);
} // void ReadRSSI

//####################################################################
// put CC1101 to receive mode
//####################################################################
void enterrx() {
  cc1101.setRxState();
  delay(2);
  rx_time = micros();
  while (((marcState = cc1101.readStatusReg(CC1101_MARCSTATE)) & 0x1F) != 0x0D )
  {
    if (micros() - rx_time > 50000) break; // Quit when marcState does not change...
  }
} // void enterrx

//####################################################################
// put CC1101 to send mode
//####################################################################
void entertx() {
  cc1101.setTxState();
  delay(2);
  rx_time = micros();
  while (((marcState = cc1101.readStatusReg(CC1101_MARCSTATE)) & 0x1F) != 0x13 && 0x14 && 0x15)
  {
    if (micros() - rx_time > 50000) break; // Quit when marcState does not change...
  }
} // void entertx

//####################################################################
// Callback for incoming MQTT messages
//####################################################################
void mqtt_callback(char* topic, byte* payload, unsigned int length) {

  // extract channel id from topic name
  int channel;
  char * token = strtok(topic, "/");
  for (; (token = strtok(NULL, "/")) != NULL; channel = atoi(token));

  // convert payload in string
  payload[length] = '\0';
  String cmd = String((char*)payload);

  // print serial message
  WriteLog("[INFO] - incoming MQTT command for channel " + (String) channel + ":", false);
  WriteLog(cmd, true);

  if (channel <= 15) {

    iset = true;
    detachInterrupt(DATAIN); // Interrupt @Inputpin
    delay(1);

    if (cmd == "UP" || cmd == "0") {
      cmd_up(channel);
    } else if (cmd == "DOWN"  || cmd == "100") {
      cmd_down(channel);
    } else if (cmd == "STOP") {
      cmd_stop(channel);
    } else if (cmd == "SETSHADE") {
      cmd_set_shade_position(channel);
    } else if (cmd == "SHADE" || cmd == "90") {
      cmd_shade(channel);
    } else if (cmd == "LEARN") {
      cmd_learn(channel);
    } else {
      WriteLog("[ERR ] - incoming MQTT topic message unknown.", true);
    }
  } else {
    WriteLog("[ERR ] - channel does not exist, choose one of 0-15", true);
  }
} // void mqtt_callback

//####################################################################
// increment and store devcnt, send devcnt as mqtt state topic
//####################################################################
void devcnt_handler(boolean do_increment = true) {
  if (do_increment)
    devcnt++;
  EEPROM.put(cntadr, devcnt);
  EEPROM.commit();
  if (mqtt_client.connected()) {
    String Topic = "stat/"+ config.mqtt_devicetopic+ "/devicecounter";
    const char * msg = Topic.c_str();
    char devcntstr[10];
    itoa(devcnt, devcntstr, 10);
    mqtt_client.publish(msg, devcntstr, true);
  }
} // void devcnt_handler

//####################################################################
// function to move the shutter up
//####################################################################
void cmd_up(int channel) {
  EEPROM.get(adresses[channel], new_serial);
  EEPROM.get(cntadr, devcnt);
  button = 0x8;
  disc_l = disc_low[channel];
  disc_h = disc_high[channel];
  disc = (disc_l << 8) | serials[channel];
  rx_disc_low[0]  = disc_l;
  rx_disc_high[0] = disc_h;
  keygen();
  keeloq();
  entertx();
  senden(2);
  enterrx();
  rx_function = 0x8;
  rx_serial_array[0] = (new_serial >> 24) & 0xFF;
  rx_serial_array[1] = (new_serial >> 16) & 0xFF;
  rx_serial_array[2] = (new_serial >> 8) & 0xFF;
  rx_serial_array[3] = new_serial & 0xFF;
  String Topic = "stat/"+ config.mqtt_devicetopic+ "/shutter/" + (String)channel;
  const char * msg = Topic.c_str();
  mqtt_client.publish(msg, "0");
  WriteLog("[INFO] - command UP for channel "+ (String)channel+ " ("+ config.channel_name[channel]+ ") sent.", true);
  devcnt_handler();
} // void cmd_up

//####################################################################
// function to move the shutter down
//####################################################################
void cmd_down(int channel) {
  EEPROM.get(adresses[channel], new_serial);
  EEPROM.get(cntadr, devcnt);
  button = 0x2;
  disc_l = disc_low[channel];
  disc_h = disc_high[channel];
  disc = (disc_l << 8) | serials[channel];
  rx_disc_low[0]  = disc_l;
  rx_disc_high[0] = disc_h;
  keygen();
  keeloq();  // Generate encrypted message 32Bit hopcode
  entertx();
  senden(2); // Call of TX routine, GPIO4 acts as output.
  enterrx();
  rx_function = 0x2;
  rx_serial_array[0] = (new_serial >> 24) & 0xFF;
  rx_serial_array[1] = (new_serial >> 16) & 0xFF;
  rx_serial_array[2] = (new_serial >> 8) & 0xFF;
  rx_serial_array[3] = new_serial & 0xFF;
  String Topic = "stat/"+ config.mqtt_devicetopic+ "/shutter/" + (String)channel;
  const char * msg = Topic.c_str();
  mqtt_client.publish(msg, "100");
  WriteLog("[INFO] - command DOWN for channel "+ (String)channel+ " ("+ config.channel_name[channel]+ ") sent.", true);
  devcnt_handler();
} // void cmd_down

//####################################################################
// function to stop the shutter
//####################################################################
void cmd_stop(int channel) {
  EEPROM.get(adresses[channel], new_serial);
  EEPROM.get(cntadr, devcnt);
  button = 0x4;
  disc_l = disc_low[channel];
  disc_h = disc_high[channel];
  disc = (disc_l << 8) | serials[channel];
  rx_disc_low[0]  = disc_l;
  rx_disc_high[0] = disc_h;
  keygen();
  keeloq();
  entertx();
  senden(2);
  enterrx();
  rx_function = 0x4;
  rx_serial_array[0] = (new_serial >> 24) & 0xFF;
  rx_serial_array[1] = (new_serial >> 16) & 0xFF;
  rx_serial_array[2] = (new_serial >> 8) & 0xFF;
  rx_serial_array[3] = new_serial & 0xFF;
  WriteLog("[INFO] - command STOP for channel "+ (String)channel+ " ("+ config.channel_name[channel]+ ") sent.", true);
  devcnt_handler();
} // void cmd_stop

//####################################################################
// function to move shutter to shade position
//####################################################################
void cmd_shade(int channel) {
  EEPROM.get(adresses[channel], new_serial);
  EEPROM.get(cntadr, devcnt);
  button = 0x4;
  disc_l = disc_low[channel];
  disc_h = disc_high[channel];
  disc = (disc_l << 8) | serials[channel];
  rx_disc_low[0]  = disc_l;
  rx_disc_high[0] = disc_h;
  keygen();
  keeloq();
  entertx();
  senden(20);
  enterrx();
  rx_function = 0x3;
  rx_serial_array[0] = (new_serial >> 24) & 0xFF;
  rx_serial_array[1] = (new_serial >> 16) & 0xFF;
  rx_serial_array[2] = (new_serial >> 8) & 0xFF;
  rx_serial_array[3] = new_serial & 0xFF;
  String Topic = "stat/"+ config.mqtt_devicetopic+ "/shutter/" + (String)channel;
  const char * msg = Topic.c_str();
  mqtt_client.publish(msg, "90");
  WriteLog("[INFO] - command SHADE for channel "+ (String)channel+ " ("+ config.channel_name[channel]+ ") sent.", true);
  devcnt_handler();
} // void cmd_shade

//####################################################################
// function to set the learn/set the shade position
//####################################################################
void cmd_set_shade_position(int channel) {
  EEPROM.get(adresses[channel], new_serial);
  EEPROM.get(cntadr, devcnt);
  button = 0x4;
  disc_l = disc_low[channel];
  disc_h = disc_high[channel];
  disc = (disc_l << 8) | serials[channel];
  rx_disc_low[0]  = disc_l;
  rx_disc_high[0] = disc_h;
  keygen();

  for (int i = 0; i < 4; i++) {
    entertx();
    keeloq();
    senden(1);
    devcnt++;
    enterrx();
    delay(300);
  }
  rx_function = 0x6;
  rx_serial_array[0] = (new_serial >> 24) & 0xFF;
  rx_serial_array[1] = (new_serial >> 16) & 0xFF;
  rx_serial_array[2] = (new_serial >> 8) & 0xFF;
  rx_serial_array[3] = new_serial & 0xFF;
  WriteLog("[INFO] - command SET SHADE for channel "+ (String)channel+ " ("+ config.channel_name[channel]+ ") sent.", true);
  devcnt_handler(false);
  delay(2000); // Safety time to prevent accidentally erase of end-points.
} // void cmd_set_shade_position

//####################################################################
// function to put the dongle into the learn mode and
// send learning packet.
//####################################################################
void cmd_learn(int channel) {
  WriteLog("[INFO] - putting channel " +  (String) channel + " into learn mode ...", false);
  new_serial = EEPROM.get(adresses[channel], new_serial);
  EEPROM.get(cntadr, devcnt);
  if (config.learn_mode == true)
    button = 0xA;                           // Regular learn method. Up+Down followd by Stop.
  else
    button = 0x1;                           // New learn method. Try if regular version does not work.
  disc_l = disc_low[channel] ;
  disc_h = disc_high[channel];
  disc = (disc_l << 8) | serials[channel];
  keygen();
  keeloq();
  entertx();
  senden(1);
  enterrx();
  devcnt++;
  if (config.learn_mode == true) {
    delay(1000);
    button = 0x4;   // Stop
    keeloq();
    entertx();
    senden(1);
    enterrx();
    devcnt++;
  }
  devcnt_handler(false);
  WriteLog("Channel learned!", true);
} // void cmd_learn

//####################################################################
// generates 16 serial numbers
//####################################################################
void cmd_generate_serials(String sn) {
  const char* serial = sn.c_str();
  WriteLog("Generate serial numbers starting from", false);
  WriteLog(sn, true);
  int z = atoi(serial);           // set serial number range
  for (int i = 0; i <= 15; ++i) { // Generation of 16 serial numbers and storage in EEPROM
    EEPROM.put(adresses[i], z);   // Serial 4Bytes
    z++;
  }
  devcnt = 0;
  devcnt_handler(false);
  delay(100);
} // void cmd_generate_serials

//####################################################################
// connect function for MQTT broker
// called from the main loop
//####################################################################
boolean mqtt_connect() {
  const char* client_id = config.mqtt_broker_client_id.c_str();
  const char* username = config.mqtt_broker_username.c_str();
  const char* password = config.mqtt_broker_password.c_str();
  String willTopic = "tele/"+ config.mqtt_devicetopic+ "/LWT"; // connect with included "Last-Will-and-Testament" message
  uint8_t willQos = 0;
  boolean willRetain = true;
  const char* willMessage = "Offline";           // LWT message says "Offline"
  String subscribeString = "cmd/"+ config.mqtt_devicetopic+ "/shutter/+";

  WriteLog("[INFO] - trying to connect to MQTT broker . . .", false);
  // try to connect to MQTT
  if (mqtt_client.connect(client_id, username, password, willTopic.c_str(), willQos, willRetain, willMessage )) {
    WriteLog("success!", true);
    // subscribe the needed topics
    mqtt_client.subscribe(subscribeString.c_str());
    // publish telemetry message "we are online now"
    mqtt_client.publish(willTopic.c_str(), "Online", true);
  } else {
    WriteLog("failed, rc ="+ (String)mqtt_client.state(), true);
  }
  return mqtt_client.connected();
} // boolean mqtt_connect
