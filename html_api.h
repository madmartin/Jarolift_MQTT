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

//####################################################################
// API call to get data or execute commands via WebIf
//####################################################################
void html_api(){
  if (server.args() > 0 )
  {
    // get server args from HTML POST
    String cmd = "";
    int channel;
    String channel_name = "";
    for ( uint8_t i = 0; i < server.args(); i++ ) {
      if (server.argName(i) == "cmd") cmd         = urldecode(server.arg(i));
      if (server.argName(i) == "channel") channel = server.arg(i).toInt();
      if (server.argName(i) == "channel_name") channel_name = urldecode(server.arg(i));

      if (server.argName(i) == "ssid") config.ssid                           = urldecode(server.arg(i));
      if (server.argName(i) == "password") config.password                   = urldecode(server.arg(i)); 
     
      if (server.argName(i) == "mqtt_broker_port") config.mqtt_broker_port            = urldecode(server.arg(i));
      if (server.argName(i) == "mqtt_broker_client_id") config.mqtt_broker_client_id  = urldecode(server.arg(i));
      if (server.argName(i) == "mqtt_broker_username") config.mqtt_broker_username    = urldecode(server.arg(i));
      if (server.argName(i) == "mqtt_broker_password") config.mqtt_broker_password    = urldecode(server.arg(i));  
          
      if (server.argName(i) == "master_msb") config.master_msb = urldecode(server.arg(i));
      if (server.argName(i) == "master_lsb") config.master_lsb = urldecode(server.arg(i));
      if (server.argName(i) == "serial") config.serial         = urldecode(server.arg(i));
        
      if (server.argName(i) == "ip_0") if (checkRange(server.arg(i)))   config.ip[0] =  server.arg(i).toInt();
      if (server.argName(i) == "ip_1") if (checkRange(server.arg(i)))   config.ip[1] =  server.arg(i).toInt();
      if (server.argName(i) == "ip_2") if (checkRange(server.arg(i)))   config.ip[2] =  server.arg(i).toInt();
      if (server.argName(i) == "ip_3") if (checkRange(server.arg(i)))   config.ip[3] =  server.arg(i).toInt();
      if (server.argName(i) == "nm_0") if (checkRange(server.arg(i)))   config.netmask[0] =  server.arg(i).toInt();
      if (server.argName(i) == "nm_1") if (checkRange(server.arg(i)))   config.netmask[1] =  server.arg(i).toInt();
      if (server.argName(i) == "nm_2") if (checkRange(server.arg(i)))   config.netmask[2] =  server.arg(i).toInt();
      if (server.argName(i) == "nm_3") if (checkRange(server.arg(i)))   config.netmask[3] =  server.arg(i).toInt();
      if (server.argName(i) == "gw_0") if (checkRange(server.arg(i)))   config.gateway[0] =  server.arg(i).toInt();
      if (server.argName(i) == "gw_1") if (checkRange(server.arg(i)))   config.gateway[1] =  server.arg(i).toInt();
      if (server.argName(i) == "gw_2") if (checkRange(server.arg(i)))   config.gateway[2] =  server.arg(i).toInt();
      if (server.argName(i) == "gw_3") if (checkRange(server.arg(i)))   config.gateway[3] =  server.arg(i).toInt();

      if (server.argName(i) == "mqtt_broker_addr_0") if (checkRange(server.arg(i)))   config.mqtt_broker_addr[0] =  server.arg(i).toInt();
      if (server.argName(i) == "mqtt_broker_addr_1") if (checkRange(server.arg(i)))   config.mqtt_broker_addr[1] =  server.arg(i).toInt();
      if (server.argName(i) == "mqtt_broker_addr_2") if (checkRange(server.arg(i)))   config.mqtt_broker_addr[2] =  server.arg(i).toInt();
      if (server.argName(i) == "mqtt_broker_addr_3") if (checkRange(server.arg(i)))   config.mqtt_broker_addr[3] =  server.arg(i).toInt();
      
      if (server.argName(i) == "dhcp") if (urldecode(server.arg(i)) == "true") { config.dhcp = true; } else { config.dhcp = false; };
      if (server.argName(i) == "learn_mode") if (urldecode(server.arg(i)) == "true") { config.learn_mode = true; } else { config.learn_mode = false; };
      
    }
    if (cmd != "")
    { 
      if (cmd == "eventlog"){
         String values ="";
         String counter;
         for( int i = 0; i < 40;  ++i ){
            if (web_log_message[i] != ""){
               if ((i+1) < 10) {
                  counter = "0" + String(i + 1);
               }else {
                  counter = String(i + 1);
               }
               values += counter + " -" +  web_log_message[i] + "\n";
            }
         }
         server.send ( 200, "text/plain", values );
      }else if (cmd == "save"){
         WriteConfig();
         server.send ( 200, "text/plain", "Configuration has been saved. System is restarting. Please refresh manually in about 30 seconds." );
         delay(500);
         ESP.restart();
      }else if (cmd == "save and generate serials"){
         web_cmd_channel = channel;
         web_cmd = cmd;
      }else if (cmd == "get channel name"){
         String values ="";
         values += "channel_0=" + config.name_c0 + "\n";
         values += "channel_1=" + config.name_c1 + "\n";
         values += "channel_2=" + config.name_c2 + "\n";
         values += "channel_3=" + config.name_c3 + "\n";
         values += "channel_4=" + config.name_c4 + "\n";
         values += "channel_5=" + config.name_c5 + "\n";
         values += "channel_6=" + config.name_c6 + "\n";
         values += "channel_7=" + config.name_c7 + "\n";
         values += "channel_8=" + config.name_c8 + "\n";
         values += "channel_9=" + config.name_c9 + "\n";
         values += "channel_10=" + config.name_c10 + "\n";
         values += "channel_11=" + config.name_c11 + "\n";
         values += "channel_12=" + config.name_c12 + "\n";
         values += "channel_13=" + config.name_c13 + "\n";
         values += "channel_14=" + config.name_c14 + "\n";
         values += "channel_15=" + config.name_c15 + "\n";
         server.send ( 200, "text/plain", values ); 

      }else if (cmd == "get config"){
         String values ="";
         values += "ssid=" + config.ssid + "\n";
         values += "password=" + config.password + "\n";
         if (config.dhcp) {values += "dhcp=1\n";}else{values += "dhcp=0\n";}
         values += "ip_0=" + (String) config.ip[0] + "\n";
         values += "ip_1=" + (String) config.ip[1] + "\n";
         values += "ip_2=" + (String) config.ip[2] + "\n";
         values += "ip_3=" + (String) config.ip[3] + "\n";
         values += "nm_0=" + (String) config.netmask[0] + "\n";
         values += "nm_1=" + (String) config.netmask[1] + "\n";
         values += "nm_2=" + (String) config.netmask[2] + "\n";
         values += "nm_3=" + (String) config.netmask[3] + "\n";
         values += "gw_0=" + (String) config.gateway[0] + "\n";
         values += "gw_1=" + (String) config.gateway[1] + "\n";
         values += "gw_2=" + (String) config.gateway[2] + "\n";
         values += "gw_3=" + (String) config.gateway[3] + "\n";
         values += "mqtt_broker_addr_0=" + (String) config.mqtt_broker_addr[0] + "\n";
         values += "mqtt_broker_addr_1=" + (String) config.mqtt_broker_addr[1] + "\n";
         values += "mqtt_broker_addr_2=" + (String) config.mqtt_broker_addr[2] + "\n";
         values += "mqtt_broker_addr_3=" + (String) config.mqtt_broker_addr[3] + "\n";
         values += "mqtt_broker_port=" + config.mqtt_broker_port + "\n";
         values += "mqtt_broker_username=" + config.mqtt_broker_username + "\n";
         values += "mqtt_broker_password=" + config.mqtt_broker_password + "\n";
         values += "mqtt_broker_client_id=" + config.mqtt_broker_client_id + "\n";
         values += "master_msb=" + config.master_msb + "\n";
         values += "master_lsb=" + config.master_lsb + "\n";
         if (config.learn_mode) {values += "learn_mode=1\n";}else{values += "learn_mode=0\n";}
         values += "serial=" + config.serial + "\n";
         server.send ( 200, "text/plain", values ); 

      }else if ( cmd == "set channel name"){
         if (channel == 0) config.name_c0 = channel_name;
         if (channel == 1) config.name_c1 = channel_name;
         if (channel == 2) config.name_c2 = channel_name;
         if (channel == 3) config.name_c3 = channel_name;
         if (channel == 4) config.name_c4 = channel_name;
         if (channel == 5) config.name_c5 = channel_name;
         if (channel == 6) config.name_c6 = channel_name;
         if (channel == 7) config.name_c7 = channel_name;
         if (channel == 8) config.name_c8 = channel_name;
         if (channel == 9) config.name_c9 = channel_name;
         if (channel == 10) config.name_c10 = channel_name;
         if (channel == 11) config.name_c11 = channel_name;
         if (channel == 12) config.name_c12 = channel_name;
         if (channel == 13) config.name_c13 = channel_name;
         if (channel == 14) config.name_c14 = channel_name;
         if (channel == 14) config.name_c15 = channel_name;
         WriteConfig();  
         String status_text = "Updating channel description to '" + channel_name + "'.";
         server.send ( 200, "text/plain", status_text ); 
         
      }else{

         web_cmd_channel = channel;
         web_cmd = cmd; 
         String status_text = "Running command '" + cmd + "' for Channel " + channel + ".";
         server.send ( 200, "text/plain", status_text ); 
      }

    }else{
      String status_text = "No Command to execute!";
      server.send ( 200, "text/plain", status_text );   
    }
  }
}
