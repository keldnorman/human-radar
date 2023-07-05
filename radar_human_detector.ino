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
 * 1 x ESP8266 with a wireless module - I use a NODE MCU AMICA = NodeMCU 1.0 (ESP-12E Module) CPU: 80 MHZ, Flash: 4M (3M SPIFFS)
 * 2 x Inductors 1mH - 420mA - 1,989Ohm - CDRH105RNP-102NC (buy more than 2 - they are very fragile )
 * 2 x Capacitors 470 uF 6volt                             (INDUCTORS)
 * 2 x Inductors 1mH - 420mA - 1,989Ohm - CDRH105RNP-102NC (COIL)
 * 1 x Micro-B cable to connect to the ESP8266
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
 * Windows people can look in their Device Manager (Press Windows button + X and select Device manager) and look under COM ports
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
 * To use the push feature ( that i made for iPhones only sorry - if you have an android use pushover instead and rewrite the code )
 * 
 * Last but not leat compile the code and upload it to the hardware:
 * 
 * Press the arrow pointing right at the top of Arduino under FILE EDIT SKETCH TOOLS HELP 
 * ( It looks like this: [->] ) 
 * Then the Arduino IDE will compile the code and upload it.
 */
//---------------------------------------------------------------------
// LIBRARIES
//---------------------------------------------------------------------
// WIFI
#include <ESP8266WiFi.h> // WiFi Hardware management
#include <WiFiManager.h> // WiFi Software management
#include <time.h>        // Time and Date
#include <stdio.h>       // Write to files and console
#include <ESP8266mDNS.h> // DNS
#include <NTPClient.h>   // NTP
#include <WiFiUdp.h>     // NTP
// FILESYSTEM
#include <FS.h>                  // Include the SPIFFS library
//---------------------------------------------------------------------
// VARIABLES
//---------------------------------------------------------------------
#define TIMER_UPDATE 1000
#define INPUT_PIN 13      // Define Pin GPIO7 as input pin ( D7 = Pin 13 )
#define TRIGGER_PIN 0     // For reset of Wifi Config
const int PIN_LED = 2;    // LED definition
const int httpsPort = 443;
const char* activity_filename = "/activity.txt";
const long utcOffsetInSeconds = 7200;  // for NTP
WiFiServer server(80);
WiFiClientSecure httpsClient; 
// TIME VARIABLES:
os_timer_t myTimer;      // Define time function for periodic read in of digital input
time_t epochTime;        // Epoch Time: 1688500218
String formattedTime;    // Formatted Time: 19:50:18
int currentHour;         // Hour: 19
int currentMinute;       // Minutes: 50
int currentSecond;       // Seconds: 18
String weekDay;          // Week Day: Tuesday
struct tm *ptm;          // [ Get a time structure ]
int monthDay;            // Month day: 4
int currentMonth;        // Month: 7   
String currentMonthName; // Month name: July
int currentYear;         // Year: 2023
unsigned long skipDuration = 1 * 60 * 1000;  // Skip duration of 1 minute (1 minute * 60 seconds * 1000 milliseconds)
unsigned long lastTriggerTime = skipDuration;
const unsigned long sendInterval = 60000;    // 1 minute minimum between notifications
// OTHER VARIABLES
bool inp = 0;            // Bolean variable for digital input reads
int i;                   // Counter
String LocalIP = "";
String Request = "";
String LogLine = "";
String HostName = "";
//---------------------------------------------------------------------
// PUSH MESSAGES VIA PROWL DEFINITIONS
//---------------------------------------------------------------------
String PROWL_API_KEY = "put-your-secret-prowl-api-key-here"; // https://www.prowlapp.com/
String PROWL_HOST = "prowl.weks.net";
String PROWL_URL = "/publicapi/add";
String PROWL_APPLICATION = "Sensor001";
String ProwlData = "";
String ProwlResponse = "";
String ProwlReturnCode = "";
long ProwlCodeValue;
int ProwlCodeIndex;
int ProwlCodeStart;
int ProwlCodeEnd;
//---------------------------------------------------------------------
// Define WiFiManager
//---------------------------------------------------------------------
WiFiManager wifiManager;
//---------------------------------------------------------------------
// NTP TIME SYNC
//---------------------------------------------------------------------
// DK
// String WeekDays[7]={"Søndag", "Mandag", "Tirsdag", "Onsdag", "Torsdag", "Fredag", "Lørdag"};
// String Months[12]={"Januar", "Februar", "Marts", "April", "Maj", "Juni", "July", "August", "September", "Oktober", "November", "December"};
// UK
String WeekDays[7]={"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
String Months[12]={"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "0.dk.pool.ntp.org", utcOffsetInSeconds);
//---------------------------------------------------------------------
// HTML CODE
//---------------------------------------------------------------------
String header = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
String html_1 = "<!DOCTYPE html>\n\
 <html>\n\
  <head>\n\
   <meta http-equiv='refresh' content='5'/>\n\
   <meta name='viewport' content='width=device-width, initial-scale=1.0'/>\n\
   <meta charset='utf-8'>\n\
   <style>\n\
    body {font-size:140%;background-color:#000000;color: white;} \n\
    #main {display: table; margin: auto;  padding: 0 10px 0 10px;}\n\
    h2,{text-align:center;}\n\
   </style>\n\
   <title>Early Warning Sensor</title>\n\
  </head>\n\
 <body>\n\
  <div id='main'>\n\
   <center>\n\
    <h2 style='color:white;'>SENSOR STATUS</h2>\n    ";
String html_2 = "";
String html_4 = "\n   </center>\n\
  </div>\n\
 </body>\n\
</html>";  
//--------------------------------------------------------------------- 
void SendProwlNotification(String event, String description, int priority) {
//--------------------------------------------------------------------- 
 ProwlData = "priority=" + String(priority) + "&application=" + PROWL_APPLICATION + "&event=" + event + "&apikey=" + PROWL_API_KEY + "&description=" + description;
 httpsClient.setInsecure();
 if (httpsClient.connect(PROWL_HOST, 443)) {
  httpsClient.println("POST " + PROWL_URL + " HTTP/1.1");
  httpsClient.println("Host: " + (String)PROWL_HOST);
  httpsClient.println("User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/74.0.3729.131 Safari/537.36");
  httpsClient.println("Content-Type: application/x-www-form-urlencoded;");
  httpsClient.print("Content-Length: ");
  httpsClient.println(ProwlData.length());
  httpsClient.println();
  httpsClient.println(ProwlData);
  ProwlResponse = httpsClient.readString().substring(ProwlResponse.indexOf("\r\n\r\n") + 4);
  ProwlCodeIndex = ProwlResponse.indexOf("code=\""); // Extract the success code from the XML response
  if (ProwlCodeIndex != -1) {
   ProwlCodeStart = ProwlCodeIndex + 6;
   ProwlCodeEnd = ProwlResponse.indexOf("\"", ProwlCodeStart);
   if (ProwlCodeEnd != -1) {
    ProwlCodeValue = ProwlResponse.substring(ProwlCodeStart, ProwlCodeEnd).toInt(); // Convert to long int
    ProwlReturnCode = String(ProwlCodeValue); // Convert to String
    if (ProwlReturnCode == "200") { // Check the success code and print the appropriate message
     Serial.println("[+] Send Push message");
    } else {
     Serial.println("[-] Failed to send Push message");
    }
   }  else {
    Serial.println("[-] Debug: CodeEnd is -1");
   }
  } else {
   Serial.println("[-] Debug: codeIndex is -1");
  }
 }
 httpsClient.stop();
}
//---------------------------------------------------------------------
void startFilesystemHandler() {
//---------------------------------------------------------------------
 // START FILESYSTEM HANDLER
 Serial.println("[+] Initializing SPIFFS filesystem..");
 if(SPIFFS.begin()) { 
   Serial.println("[+] SPIFFS Initialize....ok"); 
 } else { 
  Serial.println("[-] SPIFFS Initialization...failed");
  Serial.println("[+] Formatting SPIFFS filesystem..");
  if(SPIFFS.format()) { 
   Serial.println("[+] File System Formatted");
  } else { 
   Serial.println("[!] File System Formatting Error");
   restart_program();
  }
 }
 Serial.println(F("[+] SPIFFS filesystem handler started"));
 // Serial.println("Opening file for write..");
 File f = SPIFFS.open(activity_filename, "w"); // Initialisering af logfile
 if (!f) { 
  Serial.println("[!] Failed to create logfile!");
  f.close();
  restart_program();
 } else { 
  f.print("<br>\n");
  f.close();
 }
}
//---------------------------------------------------------------------
void get_current_time_and_date() {
//---------------------------------------------------------------------
 epochTime = timeClient.getEpochTime();          // Epoch Time: 1688500218
 formattedTime = timeClient.getFormattedTime();  // Formatted Time: 19:50:18
 currentHour = timeClient.getHours();            // Hour: 19
 currentMinute = timeClient.getMinutes();        // Minutes: 50
 currentSecond = timeClient.getSeconds();        // Seconds: 18
 weekDay = WeekDays[timeClient.getDay()];        // Week Day: Tuesday
 ptm = gmtime ((time_t *)&epochTime);            // [ Get a time structure ]
 monthDay = ptm->tm_mday;                        // Month day: 4
 currentMonth = ptm->tm_mon+1;                   // Month: 7   
 currentMonthName = Months[currentMonth-1];      // Month name: July
 currentYear = ptm->tm_year+1900;                // Year: 2023
}
//---------------------------------------------------------------------
String generateRandomString(int length) {
//---------------------------------------------------------------------
 String result = "";
 for (int i = 0; i < length; i++) {
  char randomChar = random(26) + 'A'; // Generate random ASCII character between 'A' and 'Z'
  result += randomChar;
 }
 return result;
}
//---------------------------------------------------------------------
void connectToWiFi() {
//---------------------------------------------------------------------
 bool ConnectResult;
 wifiManager.setConfigPortalTimeout(120);
 wifiManager.setConnectTimeout(20);
 wifiManager.setTimeout(60); // Sets timeout until configuration portal gets turned off
 randomSeed(analogRead(1));  // Initialize the random number generator (Read analoug noise from unconnected pin 1)
 HostName = PROWL_APPLICATION + "-" + String(generateRandomString(4));
 Serial.println("[+] Setting hostname to " + HostName);
 wifiManager.setHostname(HostName);
 wifiManager.setConnectRetries(3);
 Serial.println("[+] Connecting to WiFi");
 wifiManager.disconnect();
 ConnectResult = wifiManager.autoConnect("Sensor_Not_Configured", "password");
 if(!ConnectResult) {
    Serial.println("[!] Failed to connect");
    restart_program();
    delay(1000);
   } else {
    Serial.println("[+] Connected to WiFi.");
 }
 Serial.println("[+] My IP is: " + WiFi.localIP().toString());
 LocalIP = WiFi.localIP().toString();
 SendProwlNotification("Online", String("http://" + LocalIP) , 2);
}
//---------------------------------------------------------------------
void announceMDNS() {
//---------------------------------------------------------------------
 //Serial.println("FUNCTION: announceMDNS"); 
 if (!MDNS.begin("RadarSensor")) { 
   Serial.println("[-] Error setting up MDNS responder!"); 
 } else {
   Serial.println("[+] mDNS responder started");
 }
}
//---------------------------------------------------------------------
void timerCallback(void *pArg) { 
//---------------------------------------------------------------------
 inp = digitalRead(INPUT_PIN);
 Serial.println(inp);
 if (inp > 0) {
  if (millis() - lastTriggerTime >= skipDuration) {  // Check if the skip duration has elapsed
    lastTriggerTime = millis();  // Update the last trigger time
    digitalWrite(PIN_LED, HIGH);
    get_current_time_and_date();
    LogLine = weekDay + " " + monthDay + " " + currentMonthName + " " + currentYear + " @ " + formattedTime;
    Serial.println("[#] Activity: " + LogLine);
    File f = SPIFFS.open(activity_filename, "a");
    if (!f) {
      Serial.println("[!] Failed to write data to the logfile.");
      restart_program();
    } else {
    // Serial.println("[+] File open success");
      f.print(LogLine + String("<br>\n"));
      f.close();
    }
    Serial.println("[+] Ignoring alarms for the next 1 minute");
  }
 } else {
    digitalWrite(PIN_LED, LOW);
    LogLine = "-";
 }
}
//---------------------------------------------------------------------
void checkButton(){
//---------------------------------------------------------------------
 if ( digitalRead(TRIGGER_PIN) == LOW ) { // Check to if reset button is pressed
  delay(50);
  if( digitalRead(TRIGGER_PIN) == LOW ){
   Serial.println("[+] Button Pressed");
   delay(3000); // Still holding button after 3000 ms triggers a reset settings
   if( digitalRead(TRIGGER_PIN) == LOW ){
    Serial.println("[!] Button Held");
    Serial.println("[+] Erasing Config, restarting");
    wifiManager.disconnect();
    wifiManager.resetSettings(); // Clear all stored WiFi settings
    restart_program();
   } 
  }
 }
}
//---------------------------------------------------------------------
void restart_program() {
//---------------------------------------------------------------------
 Serial.println(); 
 Serial.println("[!] Restarting the program"); 
 ESP.restart();
}
//---------------------------------------------------------------------
void loop() {
//---------------------------------------------------------------------
 checkButton(); // Check for reset
 if (digitalRead(PIN_LED) == HIGH) {
  SendProwlNotification("Activity", LogLine, 1);
 }
 // Serial.println("FUNCTION: loop"); 
 WiFiClient client = server.available();
 if (client) {
  Request = client.readStringUntil('\r');          // Read the first line of the request
  //Serial.println("request:" + Request);
  /*
  if      ( Request.indexOf("LEDON") > 0 )  { digitalWrite(PIN_LED, HIGH); }
  else if ( Request.indexOf("LEDOFF") > 0 ) { digitalWrite(PIN_LED, LOW); }
  // Get the LED pin status and create the LED status message
  if (digitalRead(PIN_LED) == HIGH) {     // the LED is on so the button needs to say turn it off
   html_2 = "<form id='F1' action='LEDOFF'><input class='button buttonOff' type='submit' value='Turn off the LED' ></form><br>";
  } else { // the LED is off so the button needs to say turn it on
   html_2 = "<form id='F1' action='LEDON'><input class='button buttonOn' type='submit' value='Turn on the LED' ></form><br>";
  }
  */
  client.flush();
  client.print( header );
  client.print( html_1 );
  File f = SPIFFS.open(activity_filename, "r"); // Read File data
  if (!f) { 
    Serial.println("[!] Failed to write to the logfile.");
    f.close();
  } else {
   for(i=0;i<f.size();i++) {
    client.print((char)f.read()); 
   }
   f.close();
  }
  client.print(html_4);
  delay(50);
 }
 timeClient.update(); // NTP
 client.stop();
 delay(1000);
}
//---------------------------------------------------------------------
void setup() {
//---------------------------------------------------------------------
 Serial.begin(115200);
 delay(2000);
 Serial.println();
 Serial.println();
 Serial.println(F("[+] Serial started at 115200 baud"));
 startFilesystemHandler();
 connectToWiFi();
 timeClient.begin();
 // Set offset time in seconds to adjust for your timezone, for example:
 // GMT +1 = 3600
 // GMT  0 = 0
 // GMT -1 = -3600
 // timeClient.setTimeOffset(0);
 timeClient.update();
 Serial.println("[+] NTP Client started");
 announceMDNS();
 server.begin(); // START WEBSERVER
 Serial.println("[+] HTTP server started");
 // START LOOP / POLL FOR ACTIVITY
 pinMode(PIN_LED, OUTPUT);   // DEFINE LED
 pinMode(INPUT_PIN, INPUT); // Configure pin INPUT_PIN as input
 os_timer_setfn(&myTimer, timerCallback, NULL);
 os_timer_arm(&myTimer, TIMER_UPDATE, true);
}
//---------------------------------------------------------------------