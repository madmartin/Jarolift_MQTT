##Changelog

please add new changelog information here at the beginning





11/11/2017 v0.4a
  improved senden-routine to prevent wdt to reset the device and get more compatibility (controlling and learing).

02/11/2017
  added a switch (bool regular_learn = true; )to choose which learn method to use.
  Regular method is Up+Down at the same time followed by a stop.


30/10/2017
  fixed an error which prevented to learn a motor to the dongle
  reconfigured wifi as STA.

30/08/17 v0.4
  Added ESP-Reset via fhem. Added shade command to set the shutter to a predefined position.
  Position is set via the "ssp(channel)" cmd.
  Added Group Reception.

19/08/17 v0.3
   Added WiFi reconnect routine. (THX to Markus)

28/07/17 v0.2
    Modified rx/tx switch for not getting stuck (line 1182/1192)

20/07/17 v0.1
    added reception of jarolift remotes and fhem status update

