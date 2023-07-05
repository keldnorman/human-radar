# human-radar
```
This program is for creating a Human radar that can detect a present or be used as an alarm in a drawer or room.
When a present is detected it will send a push message to an iPhone 
( if you want to use it with an Android based phone then just rewrite the code to use "pushover" instead of "Prowl")

The program connects to a local WiFi but when no wifi exist it will annonce it self as an accesspoint called "Sensor_not_configured" with the password: password
When you connect to that AP you will be presented with a website at 192.168.4.1 where you via "WifiManager" can select the SSID you want the sensor 
to connect to and use for sending the alerts.

The program keeps a log on file and runs a webserver where the activity times can be seen.
The program also announces it self on the network using mDNS

ToDo:  I need to add some clean up code to the logfile so the filesystem is not filled
ToDo: I change the hostname at every boot to ensure connection to the wifi (long story dont ask ) remove it from the code and if it works for you then great.
      For some reason the wifimanager does not connect to my wifi lab if i keep the same hostname - i still get the same IP from the DHCP server with a new hostname
      and it works.. :) 
      
Pictures and more info will be added here shortly..
```
