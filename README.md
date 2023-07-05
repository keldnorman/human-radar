# human-radar

![prowlmessage](https://github.com/keldnorman/human-radar/blob/main/images/my_first_prototype.jpg?raw=true)

```
This program is for creating a Human radar that can detect a present of a human
or be used as an alarm in a drawer if it is opened or if anybody enters a room (or a secret bunker).

I am experimentating with sending the alarms via LoRa but this code here will send the alarm via WiFi to a push service
so it ends up as a push message on an iPhone.
( if you want to use it with an Android based phone then just rewrite the code to use "pushover" instead of "Prowl")
```
![prowlmessage](https://github.com/keldnorman/human-radar/blob/main/images/prowl-on-ophone.jpg?raw=true)![prowlmessage](https://github.com/keldnorman/human-radar/blob/main/images/prowl-iphone-2.jpg?raw=true)

```
The program connects to a local WiFi but when no wifi exist it will annonce it self as an accesspoint called
"Sensor_not_configured" with the password: password

When you connect to that AP you will be presented with a website at 192.168.4.1 where you via "WifiManager"
can select the SSID you want the sensor to connect to and use for sending the alerts.

The program keeps a log on file and runs a webserver where the activity times can be seen.
```
![prowlmessage](https://github.com/keldnorman/human-radar/blob/main/images/website.png?raw=true)

```
The program also announces it self on the network using mDNS

ToDo:  I need to add some clean up code to the logfile so the filesystem is not filled
ToDo: I change the hostname at every boot to ensure connection to the wifi (long story dont ask ) remove it
from the code and if it works for you then great.
For some reason the wifimanager does not connect to my wifi lab if i keep the same hostname
 - i still get the same IP from the DHCP server with a new hostname and it works.. :) 
      
```
![diagram](https://github.com/keldnorman/human-radar/blob/main/images/diagram.png?raw=true)

```
/*
 * Addition: Motion detector - Keld Norman 2019 
 * Updated:  Rewritten code  - Keld Norman (Compatible with 2023 Arduino code base)
 * Updated:  Rewritten code  - Keld Norman More stabile code and prowl alerting
 * 
 * Intended to be run on an NodeMcu ESP8266-12E 30Pin CP2102 board Amica 80 MHz 32 bit 4 MB flash 
 * OR (WeMos D1 R1 board) 
 * The radar sensor consists of a cheap Doppler Radar Motion Sensor RCWL-0516 (costs around $3)
 * A NodeMCU ESP8266 with a wireless module (costs around $5)
 * And a noise reduction circuit that prevents false positives (costs a few dollars)
 * 
 * I bought most of the hardware on aliexpress.com except from the inductors.
 *
 * Here is the hardware list: 
 *
 * 1 x ESP8266 with a wireless module - I use a NODE MCU AMICA = NodeMCU 1.0 (ESP-12E Module)
 * CPU: 80 MHZ, Flash: 4M (3M SPIFFS) (I bought it from aliexpress.com )
 * 2 x Inductors 1mH - 420mA - 1,989Ohm - Search for: CDRH105RNP-102NC (buy more than 2 - they are very fragile )
 * I bought them at Mouser Electronics, Inc. -> mouser.com
 * 2 x Capacitors 470 uF 6volt - Search for: 647-UKL0J471KPD I bought them at Mouser Electronics, Inc. -> mouser.com
 * 1 x Micro-B cable to connect to the ESP8266
```
![diagram](https://github.com/keldnorman/human-radar/blob/main/images/mouser-order.png?raw=true)

```
 *
 * Here is a diagram of the sensor, the noise reduction circuit and the ESP8266
 *  ______________________ 
 * |                      |    # You can add an 1m ohm resistor to reduce the range
 * |   Sensor RCWL-0516   |    # from 7m to 5m. See the specs on the RCWL-0516 on 
 * | _____________________|    # how to do that.
 *     |   |   |   |   |  
 *   3.3V GND OUT VID CDS 
 *         |   |   |
 *         |   D7  |
 *         |   |   |     ______     ______
 *         |   |   ---o--|COIL|--o--|COIL|--> +5v on ESP8266
 *         |   |      |          |
 *         | (*note)  = 470uf    = 470uf  <-- The two = signs here illustrates two condensators (6v or above)
 *         |          |          |
 *         -----------o----------o----------> GND on ESP8266
 *         
 *  The note:) OUT means connected to the ESP8266 to D7 (also called pin 13)
 *  
 * ARDUINO SETUP: 
 *
 * First download the arduino ide program that will compile the code and upload it to your hardware.
 * It can be downloaded here: https://www.arduino.cc/en/software
 *
 * In the Arduino IDE you need to add the drivers for the esp8266.
 * 
 * Select the FILE menu -> Preferences -> And add the following line to the "Additional Boards Manager URL":  
 * https://arduino.esp8266.com/stable/package_esp8266com_index.json
 * 
 * Then select: Tools  -> Manage Libraries
 * In the search field -> Search for wifimanager ( the one made by tzapu )
 * and install it.
 * 
 * On the same menu ( Tools -> Manage Libraries ): 
 * In the search field -> Search for ntpclient ( the one made by Fabrice Weinberg )
 * and install it.
 * 
 * Now go to the Tools -> Board Selection -> Boards Manager: 
 * In the search field -> Search for esp8266 (find the one called "by ESP8266 Community"),
 * and install it.
 *
 * Now select your hardware:
 *
 * Select Tools -> Boards -> esp8266 -> find the board called : LOLIN(WeMos) D1 R1
 * ( i am using an ESP8266-12E 30Pin CP2102 board Amica 80 MHz 32 bit 4 MB flash )
 *
 * Now select Tools -> Port -> Select the com port the esp8266 is listed as.
 * Windows people can look in their Device Manager (Press Windows button + X and select Device manager)
 * and look under COM ports
 * Linux people can look at /dev
 * Mac people .. 
 *
 * Now set the specs for your hardware:
 * 
 * Open Tools -> Board and select 
 *  Upload Speed:		    921600
 *  Debug port:		      Disable
 *  Flash Size:		      4M (FS:2MB OTA:~1019KB)
 *  C++ Exceptions:     Disabled (new aborts on oom)
 *  IwIP Variant:		    v2 lower Memory ( perhaps v2 Higher Bandwidth would also work - not testet )
 *  Debug Level:		    None
 *  MMU:                32KB cache + 32KB IRAM (balanced)
 *  Non-32-bit Accesss: Use pgm_read macros for IRAM/PROGMEM
 *  SSL Support:        All SSL Ciphers (most compatible)
 *  Stack protection:   Disabled
 *  Vtables:            Flash
 *  Erase Flash         Only Flash (set it to All Flash Contents the first time you upload the code )
 *  CPU Frequency: 	80 MHz
 *
 * To use the push feature ( that i made for iPhones only sorry.
 * If you have an android use pushover instead and rewrite the code )
 * You must go to the prowl website and create an API and on your iPhone download Prowl
 *
```
 [ * Prowls website: https://www.prowlapp.com/](https://www.prowlapp.com/) 

 [* Prowls iPhone app: https://apps.apple.com/dk/app/id320876271](https://apps.apple.com/dk/app/id320876271) 
```
 *
 * Add the API string to the code where it says: String PROWL_API_KEY = "put-your-secret-prowl-api-key-here";
 *
```
![diagram](https://github.com/keldnorman/human-radar/blob/main/images/prowl.png?raw=true)

```
 * 
 * Last but not leat compile the code and upload it to the hardware:
 * 
 * Press the arrow pointing right at the top of Arduino under FILE EDIT SKETCH TOOLS HELP 
 * ( It looks like this: [->] ) 
 * Then the Arduino IDE will compile the code and upload it.
```
![diagram](https://github.com/keldnorman/human-radar/blob/main/images/arduino.png?raw=true)

```
 */

```

