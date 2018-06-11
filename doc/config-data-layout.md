# Config Data stored in flash

Jarolift Dongle stores its config data in the ESP8266 flash storage.
In the source code, the C++ class "EEPROM" is used to manage this, therefore
this ia called "EEPROM Data"

EEPROM layout: Size is 4096 / 0x1000 Bytes

Current layout:
```
Offset  Offset  Length      Content
Decimal Hex     in Byte     Description
=================================================================================
5       0x005   3           channel[0] serialnumber - highest 3 byte of uint32_t
11      0x00b   3           channel[1] serialnumber - highest 3 byte of uint32_t
17      0x011   3           channel[2] serialnumber - highest 3 byte of uint32_t
23      0x017   3           channel[3] serialnumber - highest 3 byte of uint32_t
29      0x01d   3           channel[4] serialnumber - highest 3 byte of uint32_t
35      0x023   3           channel[5] serialnumber - highest 3 byte of uint32_t
41      0x029   3           channel[6] serialnumber - highest 3 byte of uint32_t
47      0x02f   3           channel[7] serialnumber - highest 3 byte of uint32_t
53      0x035   3           channel[8] serialnumber - highest 3 byte of uint32_t
59      0x03b   3           channel[9] serialnumber - highest 3 byte of uint32_t
65      0x041   3           channel[10] serialnumber - highest 3 byte of uint32_t
71      0x047   3           channel[11] serialnumber - highest 3 byte of uint32_t
77      0x04d   3           channel[12] serialnumber - highest 3 byte of uint32_t
85      0x055   3           channel[13] serialnumber - highest 3 byte of uint32_t
91      0x05b   3           channel[14] serialnumber - highest 3 byte of uint32_t
97      0x061   3           channel[15] serialnumber - highest 3 byte of uint32_t
                            
110     0x06e   int         cntadr Where the 16Bit counter is stored
                            is inizialized with "0" when generating new serial numbers
120     0x078   1           C  Character sequence "Cfg" to recognize config data
121     0x079   1           f
122     0x07a   1           g
123     0x07b   2 uint16_t  cfgVersion (since version=2)
128     0x080   1 boolean   true=1=dhcp
144     0x090   4           ip[0..3]
148     0x094   4           netmask[0..3]
152     0x098   4           gateway[0..3]
156     0x09c   4           mqtt_broker_addr[0..3]
164     0x0a4   32          ssid
196     0x0c4   64          password
262     0x106   6           mqtt_broker_port (as string)
270     0x10e   10          master_msb (as string)
330     0x14a   10          master_lsb (as string)
350     0x15e   1 boolean   learn_mode true=1 => new learn_mode
366     0x16e   9           serial (as string)
375     0x177   25          mqtt_broker_client_id
400     0x190   25          mqtt_broker_username
450     0x1c2   25          mqtt_broker_password
500     0x1f4   25          channel_name[0]
550     0x226   25          channel_name[1]
600     0x258   25          channel_name[2]
650     0x28a   25          channel_name[3]
700     0x2bc   25          channel_name[4]
750     0x2ee   25          channel_name[5]
800     0x320   25          channel_name[6]
850     0x352   25          channel_name[7]
900     0x384   25          channel_name[8]
950     0x3b6   25          channel_name[9]
1000    0x3e8   25          channel_name[10]
1050    0x41a   25          channel_name[11]
1100    0x44c   25          channel_name[12]
1150    0x47e   25          channel_name[13]
1200    0x4b0   25          channel_name[14]
1250    0x4e2   25          channel_name[15]
1300    0x514   20          mqtt_devicetopic
```

## Changes: 
### before Versioning => declared as Version 1
- moved mqtt_broker_port from offset x to offset 262 
  (to make room for 64 byte WLAN password)

### Version 2:
- config detection sequence changed from "CFG" to "Cfg"
- added config version number (uint16_t) at offset 123
- added mqtt_devicetopic string(20) offset 1300
- serial string(9) offset 366
  - either decimal full serial number
  - or (when leading chars are "0x") hex serial prefix 6 digit/3 byte



# dump eeprom config 

usually the NodeMCU or Wemos boards have 4M Flash. 
With a memory-setting of Flash-Size 4M/SPIFFS 1M
the config EEPROM area starts at 0x3fb000 with a length of 0x1000 byte

You can download, upload or clear this config memory with esptool.py

Examples:
```
DEVICE="/dev/ttyUSB3"

# store config to file Jarolift-Cfg.img
esptool.py --port $DEVICE read_flash 0x3fb000 0x1000 Jarolift-Cfg.img

# load config from file Jarolift-Cfg.img to EEPROM area
esptool.py --port $DEVICE write_flash 0x3fb000 Jarolift-Cfg.img

# clear config area
esptool.py --port $DEVICE erase_region 0x3fb000 0x1000
```

