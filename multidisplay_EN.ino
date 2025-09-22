//    *********************************************
//
//    TOBERS MULTIDISPLAY
//    FOR ESP8266 AND ESP32
//
//    V 1.3.7 - 21.09.2025
//
//    *********************************************
//
//    TOBERS MULTIDISPLAY, based on the fantastic Parola Library by majic Designs,
//    turns your LED-Matrix display into a widely configurable and customizable message center. 
//    It shows current time & date, weather forecasts, current news, custom messages
//    and - ESP32 only - songs currently playing on Spotify.
//    Configuration and settings are made via web interface.
//    For instructions and further information see: https://www.hackster.io/eltoberino/tobers-multidisplay-for-esp8266-and-esp32-17cac9
//
//    Copyright (c) 2020-2025 Tobias Schulz
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.

//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
//    ***************************************************************
//
//    successfully compiled with ARDUINO IDE 1.8.12 - 1.8.19
//
//    required board installation:
//    - ESP8266 core for Arduino -> https://github.com/esp8266/Arduino                 successfully compiled with V 2.6.3, 2.7.1, 2.7.4, 3.0.2, 3.1.0, 3.1.2
//
//    - ESP32 core for Arduino -> https://github.com/espressif/arduino-esp32           successfully compiled with V 1.0.4, 1.0.5, 1.0.6, 2.0.2, 2.0.5, 2.0.6, 2.0.14, 2.0.17
//      ------->  V 2.0.17 recommended due to some possibly occuring issues with httpclient in Versions >= V.3.0
//
//    required libraries:
//    - MAX72xx Library by majicDesigns -> https://github.com/MajicDesigns/MD_MAX72XX             successfully compiled with V 3.5.1
//    - Parola Library by majicDesigns -> https://github.com/MajicDesigns/MD_Parola               successfully compiled with V 3.7.3
//    - Arduino Json library by Benoit Blanchon -> https://github.com/bblanchon/ArduinoJson       successfully compiled with V 7.4.2
//    - my fork of WifiManager library (development branch) by tzapu/tablatronix -> https://github.com/ElToberino/WiFiManager_for_Multidisplay
//
//    reqired accounts/api keys:
//    - WEATHER: personal api key from https://openweathermap.org/
//    - NEWS: personal api key from https://newsapi.org/
//    - SPOTIFY: (free or premium) account AND developer registration of your device -> https://developer.spotify.com/dashboard/
//
//    required files:
//    - all files delivered with this ino.file are required! -> the files in folder data must be uploaded on SPIFFS
//
//    ****************************************************************
//
//    Credits:
//    - Special thanks to Marco Colli (MajicDesigns) for his libraries, the excellent documentation and the support via arduino forum.
//    - Special thanks to Benoit Blanchon for his great Arduino Json Library and his friendly support.
//    - Local language concept and some parts of weather functions inspired by ericBcreator -> https://www.hackster.io/ericBcreator/1024-led-matrix-wifi-message-board-with-menu-web-interface-1b2666
//    - SPIFFS administration taken and adopted from the great Arduino ESP website https://fipsok.de/ (Jens Fleischer)
//    - HTML background pattern graphic by Henry Daubrez, taken from http://thepatternlibrary.com/
//    - Thanks to the many, many other programmers and enthusiasts whose work and helpfulness enabled me to realize such a project
//
//    ***************************************************************
//
//    PLEASE NOTICE THE COMMENTS and set the #defines and hardware configuration parameters according to your needs.
//
//    ***************************************************************
//
//    CHANGELOG V 1.3.6 -> V 1.3.7:  - changes in Spotify authentication process due to Spotify's currently changed policy for redirect URIs
//                                   - minor fixes in getNewsData() & showAdvanced()
//                                   - new variable "previoustimecall" for timeOnly mode used in displayOnlyTime()
//                                    
//                                                               
//    ***************************************************************




/// SETTINGS ///

//#define ESP32                                   // define for ESP32, leave commented out for ESP8266

//#define SPOTIFY                                 // works only on ESP32; ESP8266 crashes due to lack of heap on establishing https connection

//#define  DEBUG                                  // show debug messages in serial monitor
                                                    // NOTES only for ESP8266: - with DEBUG active, in some (special and rare) situations loading weather data can fail due to lack of heap 


//#define  LOCAL_LANG                             // enable local language
#define  OTA                                    // enables Over the Air Update (OTA) via web browser



#include <MD_Parola.h>                          // Parola Library by majicDesigns -> https://github.com/MajicDesigns/MD_Parola
#include <MD_MAX72xx.h>                         // MAX72xx Library by majicDesigns -> https://github.com/MajicDesigns/MD_MAX72XX
#include <ArduinoJson.h>                        // Arduino Json library by Benoit Blanchon -> https://github.com/bblanchon/ArduinoJson
#include <SPI.h>
#include <WiFiClient.h>
#include <FS.h>
#include <WiFiManager.h>                        // WiFiManager_for_Multidisplay, my fork of WifiManager library by tzapu/tablatronix -> https://github.com/ElToberino/WiFiManager_for_Multidisplay
#include <DNSServer.h>



#ifdef ESP32
  #include <WiFi.h>                              
  #include <WebServer.h>
  #include <HTTPClient.h>                      
  #include <SPIFFS.h>
  #include <Update.h>
 #ifdef SPOTIFY
  #include <base64.h>
 #endif


#else
  #include <ESP8266WiFi.h>
  #include <ESP8266WebServer.h>
  #include <ESP8266HTTPClient.h>
  #include <time.h>
#endif


#include "fontfiles/Font_Data_Numeric.h"                  // local file: Parola font set with special numeric characters only for displaying time
#include "fontfiles/Font_Data_Tob_UTF8.h"                 // local file: Parola font set with support of special characters

#define ARRAY_SIZE(x)  (sizeof(x)/sizeof(x[0]))



//// PIN DEFINITIONS ////

#ifdef ESP32
    //PINS ESP32
  #define CLK_PIN   18   // CLK                     
  #define DATA_PIN  23   // MOSI                        
  #define CS_PIN     5   // CS
#else
   //PINS ESP8266
  #define CLK_PIN   14   // CLK
  #define DATA_PIN  13   // MOSI
  #define CS_PIN    15   // CS
#endif



///// PAROLA DISPLAY ////

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW       // define LED Matrix Hardware  (options: PAROLA_HW, GENERIC_HW, ICSTATION_HW, FC16_HW) see: https://forum.arduino.cc/index.php?topic=560635.0 
#define MAX_DEVICES 8

MD_Parola P = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);



//// WIFI ////

const char* AP_NAME = "Tobers_Multidisplay";          // Parameters of wifimanager configportal and/or operation as Access Point without WiFi connection
const char* AP_PW = "";                               // set a password here if you don't want the AP to be open -> at least 8 characters !

IPAddress AP_IP(192,168,4,1);                         // NOTE: This manual IP-Setting doesn't work with ESP32 if SPOTIFY is defined! Default IP is 192.168.4.1
IPAddress AP_Netmask(255,255,255,0);
uint8_t static_Mode_enabled = 0;

bool wifiManagerWasCalled = false;                    // is set to true if WifiManager is executed on startup
bool AP_established = false;                          // is set to true if WifiManager was skipped anddevice operates as AP


///// TIME SERVER AND DATE ////

const char* timezone =  "CET-1CEST,M3.5.0/02,M10.5.0/03";       // = CET/CEST  --> for adjusting your local time zone see: https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv
const char* ntpServer = "europe.pool.ntp.org";                         //pool.ntp.org       // server pool prefix "2." could be necessary with IPv6

struct tm tm;
extern "C" uint8_t sntp_getreachability(uint8_t);               // shows reachability of NTP Server (value != 0 means server could be reached) see explanation in http://savannah.nongnu.org/patch/?9581#comment0:

#ifdef LOCAL_LANG
  const char* const PROGMEM days[] { "Sonntag", "Montag", "Dienstag", "Mittwoch", "Donnerstag", "Freitag", "Samstag" } ;
  const char* const PROGMEM months[] { "Jan", "Feb", "Mrz", "Apr", "Mai", "Jun", "Jul", "Aug", "Sep", "Okt", "Nov", "Dez" } ;
  const char* const PROGMEM monthsXL[] { "Jänner", "Februar", "März", "April", "Mai", "Juni", "Juli", "August", "September", "Oktober", "November", "Dezember" } ;
#else
  const char* const PROGMEM days[] { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" } ;
  const char* const PROGMEM months[] { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" } ;
  const char* const PROGMEM monthsXL[] { "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December" } ;
#endif

uint8_t enableTime = 1;                    // internal variable set via webinterface
uint8_t enableDate = 1;                    // internal variable set via webinterface

unsigned long previoustimecall = 0;        // internal variable needed for refresh timer in timeOnly mode

//#define SHORTDATE                        // comment this out if you prefer a shorter date display: "1 Mar 2020" instead of "Sunday 1 March 2020" 



//// WEATHER API ////                     // Writes results in table "maintable.wetcur" 
                                          //--> If numberofcities*2 > (numofarticles * newssources), size of maintable must be changed in variable tablesize!

const String WeatherApiKey = "XXXXXXXXXX";        // personal api key necessary for retrieving the weather data
bool imperial = false;                                                  // set to true if you want to receive weather data with imperal unit for temperature "Fahrenheit". Default is metric "Celsius".
const String ownCityIDs[] = {                                           // Search your city at https://openweathermap.org/ and grab the ID from URL. Example: https://openweathermap.org/city/2761369 --> ID=2761369
  "2761369",    // Vienna
  "2643743"     // London
  };


bool weathEverLoaded = false;                                           // is set to true if getWeatherData() is executed for the first time
uint8_t enableWeath = 1;                                                // internal variable set via webinterface
uint8_t WeatherCityLoop = 0;                                            // internal variable
const uint8_t numberofcities = (sizeof (ownCityIDs)/sizeof (ownCityIDs[0]));              // number of cities in array ownCityIDs[]   see: https://forum.arduino.cc/index.php?topic=420324.0
unsigned long previousweathercall = 0;                                                    // internal variable needed for refresh timer
char dayAfterTomorrow[12];                                                                // shows the day after tomorrow in weather forecast

#ifdef LOCAL_LANG 
  const char* const PROGMEM windDirection[] {
    "Nord", "Nord-Nordost", "Nordost", "Ost-Nordost", "Ost", "Ost-Südost", "Südost", "Süd-Südost",
    "Süd", "Süd-Südwest", "Südwest", "West-Südwest", "West", "West-Nordwest", "Nordwest", "Nord-Nordwest"
  };
  const String weatherLanguage = "DE";                                  // define local language for openweather
#else
  const char* const PROGMEM windDirection[] {
    "north", "north-northeast", "northeast", "east-northeast", "east", "east-southeast", "southeast", "south-southeast",
    "south", "south-southwest", "southwest", "west-southwest", "west", "west-northwest", "northwest", "north-northwest"
  };
  const String weatherLanguage = "EN";
#endif




////   NEWS API   ////                                              // Writes results in table "maintable.newscur" and "maintable.newslink" 
                                                                    // --> If "numofarticles" and/or "newssources" are changed, number of elements in "maintable" must be adopted
                                                                    //     and functions getNews() and composeNews() and news.html must be changed

const String NewsApiKey = "XXXXXXXXXX";       // your personal news api key

const String ownNewsSources[] = {                                   // your inidividual news sources -> part of the url used for newsserver call in getNewsData(); for details see https://newsapi.org/docs
  "top-headlines?sources=bbc-news",                                 // IMPORTANT NOTE -> changes of number of elements require changes in functions getNews(), composeNews() and on news.html site !!!
  "everything?domains=politico.eu",
  "top-headlines?sources=cnn",
  "everything?domains=washingtonpost.com"
};


bool newsEverLoaded = false;                                        // is set to true if getWeatherData() is executed for the first time
uint8_t enableNews1 = 1;                                            // internal variable set via webinterface
uint8_t enableNews2 = 1;                                            // internal variable set via webinterface
uint8_t enableNewsXL = 1;                                           // value 1 enables larger news arrays including news description; is set via webinterace config site
const uint8_t newssources = (sizeof (ownNewsSources)/sizeof (ownNewsSources[0]));              // number of newssources in array ownNewsSources[] and number of api requests per call of function
const uint8_t numofarticles = 3;                                    // number of articles per news source -> change of this value requires changes in funcion getNewsData()
unsigned long previousnewscall = 0;                                 // needed for refresh timer


const uint8_t tablesize = numofarticles*newssources;               // (number of articles * number of newssources) defines size of maintable
                                                                   // change to "const uint8_t tablesize = numberofcities*2" if (numberofcities*2) > (numofarticles*newssources)



////  SPOTIFY  ////

uint8_t enableSpotify = 1;                                           // internal variable set via webinterface, enables calling Spotify for information about currently playing song (only ifdef Spotify)

#ifdef SPOTIFY
const char* clientID = "XXXXXXXXXX";                                      // register your device on https://developer.spotify.com/dashboard/
const char* clientSecret = "XXXXXXXXXX";

const char* redirectUri = "http://127.0.0.1:80/callback";
                                                                                                // certificate necessary for https connection, must be updated if expired. 
                                                                                                // certificate can be found by clicking the lock symbol in address field of your browser calling "spotify.com"
                                                                                                //  --> used certificate "DigiCert Global Root G2" see: https://www.digicert.com/kb/digicert-root-certificates.htm
const char PROGMEM digicert_CA_pem[] = R"%%%(
-----BEGIN CERTIFICATE-----
MIIDjjCCAnagAwIBAgIQAzrx5qcRqaC7KGSxHQn65TANBgkqhkiG9w0BAQsFADBh
MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3
d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBH
MjAeFw0xMzA4MDExMjAwMDBaFw0zODAxMTUxMjAwMDBaMGExCzAJBgNVBAYTAlVT
MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j
b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IEcyMIIBIjANBgkqhkiG
9w0BAQEFAAOCAQ8AMIIBCgKCAQEAuzfNNNx7a8myaJCtSnX/RrohCgiN9RlUyfuI
2/Ou8jqJkTx65qsGGmvPrC3oXgkkRLpimn7Wo6h+4FR1IAWsULecYxpsMNzaHxmx
1x7e/dfgy5SDN67sH0NO3Xss0r0upS/kqbitOtSZpLYl6ZtrAGCSYP9PIUkY92eQ
q2EGnI/yuum06ZIya7XzV+hdG82MHauVBJVJ8zUtluNJbd134/tJS7SsVQepj5Wz
tCO7TG1F8PapspUwtP1MVYwnSlcUfIKdzXOS0xZKBgyMUNGPHgm+F6HmIcr9g+UQ
vIOlCsRnKPZzFBQ9RnbDhxSJITRNrw9FDKZJobq7nMWxM4MphQIDAQABo0IwQDAP
BgNVHRMBAf8EBTADAQH/MA4GA1UdDwEB/wQEAwIBhjAdBgNVHQ4EFgQUTiJUIBiV
5uNu5g/6+rkS7QYXjzkwDQYJKoZIhvcNAQELBQADggEBAGBnKJRvDkhj6zHd6mcY
1Yl9PMWLSn/pvtsrF9+wX3N3KjITOYFnQoQj8kVnNeyIv/iPsGEMNKSuIEyExtv4
NeF22d+mQrvHRAiGfzZ0JFrabA0UWTW98kndth/Jsw1HKj2ZL7tcu7XUIOGZX1NG
Fdtom/DzMNU+MeKNhJ7jitralj41E6Vf8PlwUHBHQRFXGU7Aj64GxJUTFy8bJZ91
8rGOmaFvE7FBcf6IKshPECBV1/MUReXgRPTqh5Uykw7+U0b6LJ3/iyK5S9kJRaTe
pLiaWN0bfVKfjllDiIGknibVb63dDcY3fe0Dkhvld1927jyNxF1WW6LZZm6zNTfl
MrY=
-----END CERTIFICATE-----
)%%%";


String refreshtoken;                                                                            // necessary for getting access token
String auth;                                                                                    // base64 encoded "client_id:client_secret"
String accesstoken;                                                                             // necessary for calling information about currently playing song, valid until expire time has runs out
uint16_t expiretime = 1;                                                                        // expire time is set with every call for a new access token
unsigned long previousrefresh = 0;                                                              // needed for refresh timer
#endif



////  WEBSERVER  ////

#ifdef ESP32
  WebServer server(80);
#else
  ESP8266WebServer server(80);
#endif

const char* www_username = "multi";                    // credendtials for html admin sites -> must be entered in browser to reach these sites
const char* www_password = "display";                   



////  OTA  ////                                        // messages sent to browser if OTA successful or failed

#ifdef OTA
  #ifdef LOCAL_LANG
    const char OTAsuccessMsg[] PROGMEM = "<div style=\"border:0.1em solid #e1c3a9; margin:0; padding:0.5em;font-size:0.8em; font-family:sans-serif; text-align:center; color:white; background-color:green;\">Update erfolgreich</span>";
    const char OTAfailMsg[] PROGMEM = "<div style=\"border:0.1em solid #e1c3a9; margin:0; padding:0.5em;font-size:0.8em; font-family:sans-serif; text-align:center; color:white; background-color:red;\">Update fehlgeschlagen</span>";
  #else
    const char OTAsuccessMsg[] PROGMEM = "<div style=\"border:0.1em solid #e1c3a9; margin:0; padding:0.5em;font-size:0.8em; font-family:sans-serif; text-align:center; color:white; background-color:green;\">Update successful</span>";
    const char OTAfailMsg[] PROGMEM = "<div style=\"border:0.1em solid #e1c3a9; margin:0; padding:0.5em;font-size:0.8em; font-family:sans-serif; text-align:center; color:white; background-color:red;\">Update failed</span>";
  #endif
#endif



//// GLOBALS ////

// Message arrays

#define MSG_SIZE 500                   // global size for all custom messages

char infowifi[100];
char timeshow[10];                     // array for time shown on display
char dateshow[30];                     // array for date shown on display
char showweather[500];                 // array for weather shown on display
char shownews[500];                    // array for news shown on display
char shownews2[500];                   // array for news shown on display
char pd_url[200];                      // array for news url shown internally and sent to html page

char spotify[500] = "Spotify is deactivated";           // array for Spotify shown internally and sent to html page
char showspotify[500] = "...";                                      // array for Spotify shown on display
char spotifycover[200] = "/noSpot.jpg";    // array for Spotify cover urls shown internally and sent to html page

char ownmessage[MSG_SIZE];             //array for own messages shown internally and sent to html page
char showownmessage[MSG_SIZE];         //array for own messages shown on display

char messages[4][MSG_SIZE] = {"Message A","Message B","Message C", "Message D"};          // customizable messages array shown internally and sent to html page
char showmessages[4][MSG_SIZE] = {"Message A","Message B","Message C", "Message D"};      // customizable messages array shown on display
uint8_t enablemessage[4] = { 1, 1, 1, 1};                                                 // internal variables set via webinterface enable custom messages

uint8_t enableOwn = 1;                    // internal variable set via webinterface enables public user defined message
bool changeflag = true;                   // states if ownmessage has been deactivated during runtime


// Misc

uint16_t delayTime = 1000;                      // multiplier for delay before animation in ms
uint8_t speedTime = 5;                          // multiplier for animation speed in ms (time between frames)  -> higher speed time means slower animation
uint16_t pauseTime = 500;                       // multiplier for pause of animation in ms (time last frame stays on display)
      
uint8_t intens = 1;                             // variable for display intensity
uint8_t globScrollSpeed = 4;                    // variable for global scroll speed (Infowifi, Date, News, Weather, Spotify, Ownmessage);
bool timeOnly = false;                          // if true, display shows only time


// Own characters

uint8_t degreeC[] = {9, 3, 3, 0, 62, 65, 65, 65, 65, 0 };   // Degree Celsius character     // online font-editor available at: https://pjrp.github.io/MDParolaFontEditor  
uint8_t degreeF[] = { 9, 3, 3, 0, 127, 9, 9, 9, 1, 0 };     // Degree Fahrenheit character
uint8_t degreeC_ascii = 0x8F;                      // defines position 143 in UTF-8 table
uint8_t degreeF_ascii = 0x90;                      // defines position 144 in UTF-8 table


// Table for news and weather messages

struct maintable                        // table for weather and news
{
 char wetcur[500];                      // current weather strings
 char newscur[500];                     // news strings
 char newslink[200];                    // news url strings
};

maintable table[tablesize];             // number of elements specified by (newssources * numofarticles) or (numberofcities*2)  - depending on which number is larger




//--------------------------------------------------------------------------------------

/// MAIN ARRAY CALLED IN LOOP AND SHOWN BY DISPLAY ///
 
struct sCatalog                                                            
{
  uint8_t  effectin;           // text effect in
  char*    psz;                // text string
  uint8_t  effectout;          // text effect out
  uint8_t  just;               // text alignment
  uint16_t speed;              // speed multiplier of library default
  uint16_t pause;              // pause multiplier of library default
  uint16_t delay;              // delay multiplier before next message
  };

sCatalog  catalog[] =                                      //changes of number and order of elements can require changes in array location[] !
{
  { 3, infowifi, 3 , 1, globScrollSpeed, 4, 1},                          
  { 1, timeshow, 1 , 1, 2, 6, 1},                          // DON'T CHANGE any values here in catalog[] !!!
  { 3, dateshow, 3 , 1, 2, 0, 1},                          // values of custom messages are loaded on startup from messages.txt saved on SPIFFS
  { 1, showmessages[0], 1, 1, 1, 1, 1},                    // values of other messages can be adopted in function enableOrDisable()
  { 3, showweather, 3 , 1, 2, 0, 1},
  { 3, shownews, 3 , 1, 2, 0, 1},
  { 1, showmessages[1], 1, 1, 1, 1, 1},
  { 28, showmessages[2], 28, 1, 1, 1, 1},
  { 31, showmessages[3], 31, 1, 1, 1, 1},
  { 3, showspotify, 3 , 1, 2, 0, 0},
  { 3, shownews2, 3 , 1, 2, 0, 1},
  { 3, showownmessage, 3, 1, 2, 0, 1}
   };


uint8_t location[4] = {3,6,7,8};                              // Location of showmessages in Main Array
uint8_t locationTime = 1;                                     // Location of timeshow in Main Array
uint8_t locationDate = 2;                                     // Location of dateshow in Main Array
uint8_t locationWeath = 4;                                    // Location of showweather in Main Array
uint8_t locationNews[2] = {5,10};                             // Location of shownews in Main Array
uint8_t locationSpot = 9;                                     // Location of showspotify in Main Array
uint8_t locationOwn = 11;                                     // Location of showownmessage in Main Array


struct sCatalogTEMP                               // holds custom messages when custom message[i] is deactivated                                                          
{
  uint8_t    effectinTEMP;       // text effect in
  char *          pszTEMP;       // text string
  uint8_t   effectoutTEMP;       // text effect out
  uint8_t        justTEMP;       // text alignment
  uint16_t      speedTEMP;       // speed multiplier of library default
  uint16_t      pauseTEMP;       // pause multiplier of library default
  uint16_t      delayTEMP;       // delay multiplier before next message
  };

sCatalogTEMP  catalogTEMP[] =                                      //changes of number and order of elements can require changes in array location[] !
{
  { 1, messages[0], 1, 1, 1, 1, 1},
  { 1, messages[1], 1, 1, 1, 1, 1},
  { 28, messages[2], 28, 1, 1, 1, 1},
  { 31, messages[3], 31, 1, 1, 1, 1},
};



textEffect_t effectlist[] = {                                 // List of in and out effects of Parola library
   PA_PRINT,                   // 0
   PA_SCROLL_UP,               // 1
   PA_SCROLL_DOWN,             // 2
   PA_SCROLL_LEFT,             // 3
   PA_SCROLL_RIGHT,            // 4
   PA_SLICE,                   // 5
   PA_MESH,                    // 6
   PA_FADE,                    // 7
   PA_DISSOLVE,                // 8
   PA_BLINDS,                  // 9
   PA_WIPE,                    // 10
   PA_WIPE_CURSOR,             // 11
   PA_SCAN_HORIZ,              // 12
   PA_SCAN_HORIZX,             // 13
   PA_SCAN_VERT,               // 14
   PA_SCAN_VERTX,              // 15
   PA_OPENING,                 // 16
   PA_OPENING_CURSOR,          // 17
   PA_CLOSING,                 // 18
   PA_CLOSING_CURSOR,          // 19
   PA_SCROLL_UP_LEFT,          // 20
   PA_SCROLL_UP_RIGHT,         // 21
   PA_SCROLL_DOWN_LEFT,        // 22
   PA_SCROLL_DOWN_RIGHT,       // 23
   PA_GROW_UP,                 // 24
   PA_GROW_DOWN,               // 25
   PA_NO_EFFECT,               // 26
   PA_RANDOM,                  // 27
   PA_SPRITE,                  // 28 WALKER
   PA_SPRITE,                  // 29 INVADER 
   PA_SPRITE,                  // 30 CHEVRON
   PA_SPRITE,                  // 31 HEART
   PA_SPRITE,                  // 32 ARROW 1
   PA_SPRITE,                  // 33 STEAMBOAT
   PA_SPRITE,                  // 34 FBALL
   PA_SPRITE,                  // 35 ROCKET
   PA_SPRITE,                  // 36 ROLL2
   PA_SPRITE,                  // 37 PMAN2
   PA_SPRITE,                  // 38 LINES
   PA_SPRITE,                  // 39 ROLL1
   PA_SPRITE,                  // 40 SAILBOAT
   PA_SPRITE,                  // 41 ARROW2
   PA_SPRITE,                  // 42 WAVE1
   PA_SPRITE                   // 43 PMAN1
};


textPosition_t justlist[] = {
PA_LEFT,            // 0
PA_CENTER,          // 1
PA_RIGHT            // 2
};


//------------------------------------------------------------------------------------------------


//// Sprite Definitions ////

const uint8_t F_PMAN1 = 6;
const uint8_t W_PMAN1 = 8;
const uint8_t PROGMEM pacman1[F_PMAN1 * W_PMAN1] =  // gobbling pacman animation
{
  0x00, 0x81, 0xc3, 0xe7, 0xff, 0x7e, 0x7e, 0x3c,
  0x00, 0x42, 0xe7, 0xe7, 0xff, 0xff, 0x7e, 0x3c,
  0x24, 0x66, 0xe7, 0xff, 0xff, 0xff, 0x7e, 0x3c,
  0x3c, 0x7e, 0xff, 0xff, 0xff, 0xff, 0x7e, 0x3c,
  0x24, 0x66, 0xe7, 0xff, 0xff, 0xff, 0x7e, 0x3c,
  0x00, 0x42, 0xe7, 0xe7, 0xff, 0xff, 0x7e, 0x3c,
};

const uint8_t F_PMAN2 = 6;
const uint8_t W_PMAN2 = 18;
const uint8_t PROGMEM pacman2[F_PMAN2 * W_PMAN2] =  // ghost pursued by a pacman
{
  0x00, 0x81, 0xc3, 0xe7, 0xff, 0x7e, 0x7e, 0x3c, 0x00, 0x00, 0x00, 0xfe, 0x7b, 0xf3, 0x7f, 0xfb, 0x73, 0xfe,
  0x00, 0x42, 0xe7, 0xe7, 0xff, 0xff, 0x7e, 0x3c, 0x00, 0x00, 0x00, 0xfe, 0x7b, 0xf3, 0x7f, 0xfb, 0x73, 0xfe,
  0x24, 0x66, 0xe7, 0xff, 0xff, 0xff, 0x7e, 0x3c, 0x00, 0x00, 0x00, 0xfe, 0x7b, 0xf3, 0x7f, 0xfb, 0x73, 0xfe,
  0x3c, 0x7e, 0xff, 0xff, 0xff, 0xff, 0x7e, 0x3c, 0x00, 0x00, 0x00, 0xfe, 0x7b, 0xf3, 0x7f, 0xfb, 0x73, 0xfe,
  0x24, 0x66, 0xe7, 0xff, 0xff, 0xff, 0x7e, 0x3c, 0x00, 0x00, 0x00, 0xfe, 0x7b, 0xf3, 0x7f, 0xfb, 0x73, 0xfe,
  0x00, 0x42, 0xe7, 0xe7, 0xff, 0xff, 0x7e, 0x3c, 0x00, 0x00, 0x00, 0xfe, 0x7b, 0xf3, 0x7f, 0xfb, 0x73, 0xfe,
};

const uint8_t F_WAVE = 14;
const uint8_t W_WAVE = 14;
const uint8_t PROGMEM wave[F_WAVE * W_WAVE] =  // triangular wave / worm
{
  0x08, 0x04, 0x02, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x40, 0x20, 0x10,
  0x10, 0x08, 0x04, 0x02, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x40, 0x20,
  0x20, 0x10, 0x08, 0x04, 0x02, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x40,
  0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80,
  0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40,
  0x40, 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20,
  0x20, 0x40, 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01, 0x02, 0x04, 0x08, 0x10,
  0x10, 0x20, 0x40, 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01, 0x02, 0x04, 0x08,
  0x08, 0x10, 0x20, 0x40, 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01, 0x02, 0x04,
  0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01, 0x02,
  0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01,
  0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02,
  0x02, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x40, 0x20, 0x10, 0x08, 0x04,
  0x04, 0x02, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x40, 0x20, 0x10, 0x08,
};

const uint8_t F_ROLL1 = 4;
const uint8_t W_ROLL1 = 8;
const uint8_t PROGMEM roll1[F_ROLL1 * W_ROLL1] =  // rolling square
{
  0xff, 0x8f, 0x8f, 0x8f, 0x81, 0x81, 0x81, 0xff,
  0xff, 0xf1, 0xf1, 0xf1, 0x81, 0x81, 0x81, 0xff,
  0xff, 0x81, 0x81, 0x81, 0xf1, 0xf1, 0xf1, 0xff,
  0xff, 0x81, 0x81, 0x81, 0x8f, 0x8f, 0x8f, 0xff,
};

const uint8_t F_ROLL2 = 4;
const uint8_t W_ROLL2 = 8;
const uint8_t PROGMEM roll2[F_ROLL2 * W_ROLL2] =  // rolling octagon
{
  0x3c, 0x4e, 0x8f, 0x8f, 0x81, 0x81, 0x42, 0x3c,
  0x3c, 0x72, 0xf1, 0xf1, 0x81, 0x81, 0x42, 0x3c,
  0x3c, 0x42, 0x81, 0x81, 0xf1, 0xf1, 0x72, 0x3c,
  0x3c, 0x42, 0x81, 0x81, 0x8f, 0x8f, 0x4e, 0x3c,
};

const uint8_t F_LINES = 3;
const uint8_t W_LINES = 8;
const uint8_t PROGMEM lines[F_LINES * W_LINES] =  // spaced lines
{
  0xff, 0xff, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00,
  0xff, 0xff, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00,
  0xff, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff,
};

const uint8_t F_ARROW1 = 3;
const uint8_t W_ARROW1 = 10;
const uint8_t PROGMEM arrow1[F_ARROW1 * W_ARROW1] =  // arrow fading to center
{
  0x18, 0x3c, 0x7e, 0xff, 0x7e, 0x00, 0x00, 0x3c, 0x00, 0x00,
  0x18, 0x3c, 0x7e, 0xff, 0x00, 0x7e, 0x00, 0x00, 0x18, 0x00,
  0x18, 0x3c, 0x7e, 0xff, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x18,
};

const uint8_t F_ARROW2 = 3;
const uint8_t W_ARROW2 = 9;
const uint8_t PROGMEM arrow2[F_ARROW2 * W_ARROW2] =  // arrow fading to outside
{
  0x18, 0x3c, 0x7e, 0xe7, 0x00, 0x00, 0xc3, 0x00, 0x00,
  0x18, 0x3c, 0x7e, 0xe7, 0xe7, 0x00, 0x00, 0x81, 0x00,
  0x18, 0x3c, 0x7e, 0xe7, 0x00, 0xc3, 0x00, 0x00, 0x81,
};

const uint8_t F_SAILBOAT = 1;
const uint8_t W_SAILBOAT = 11;
const uint8_t PROGMEM sailboat[F_SAILBOAT * W_SAILBOAT] =  // sail boat
{
  0x10, 0x30, 0x58, 0x94, 0x92, 0x9f, 0x92, 0x94, 0x98, 0x50, 0x30,
};

const uint8_t F_STEAMBOAT = 2;
const uint8_t W_STEAMBOAT = 11;
const uint8_t PROGMEM steamboat[F_STEAMBOAT * W_STEAMBOAT] =  // steam boat
{
  0x10, 0x30, 0x50, 0x9c, 0x9e, 0x90, 0x91, 0x9c, 0x9d, 0x90, 0x71,
  0x10, 0x30, 0x50, 0x9c, 0x9c, 0x91, 0x90, 0x9d, 0x9e, 0x91, 0x70,
};

const uint8_t F_HEART = 5;
const uint8_t W_HEART = 9;
const uint8_t PROGMEM heart[F_HEART * W_HEART] =  // beating heart
{
  0x0e, 0x11, 0x21, 0x42, 0x84, 0x42, 0x21, 0x11, 0x0e,
  0x0e, 0x1f, 0x33, 0x66, 0xcc, 0x66, 0x33, 0x1f, 0x0e,
  0x0e, 0x1f, 0x3f, 0x7e, 0xfc, 0x7e, 0x3f, 0x1f, 0x0e,
  0x0e, 0x1f, 0x33, 0x66, 0xcc, 0x66, 0x33, 0x1f, 0x0e,
  0x0e, 0x11, 0x21, 0x42, 0x84, 0x42, 0x21, 0x11, 0x0e,
};

const uint8_t F_INVADER = 2;
const uint8_t W_INVADER = 10;
const uint8_t PROGMEM invader[F_INVADER * W_INVADER] =  // space invader
{
  0x0e, 0x98, 0x7d, 0x36, 0x3c, 0x3c, 0x36, 0x7d, 0x98, 0x0e,
  0x70, 0x18, 0x7d, 0xb6, 0x3c, 0x3c, 0xb6, 0x7d, 0x18, 0x70,
};

const uint8_t F_ROCKET = 2;
const uint8_t W_ROCKET = 11;
const uint8_t PROGMEM rocket[F_ROCKET * W_ROCKET] =  // rocket
{
  0x18, 0x24, 0x42, 0x81, 0x99, 0x18, 0x99, 0x18, 0xa5, 0x5a, 0x81,
  0x18, 0x24, 0x42, 0x81, 0x18, 0x99, 0x18, 0x99, 0x24, 0x42, 0x99,
};

const uint8_t F_FBALL = 2;
const uint8_t W_FBALL = 11;
const uint8_t PROGMEM fireball[F_FBALL * W_FBALL] =  // fireball
{
  0x7e, 0xab, 0x54, 0x28, 0x52, 0x24, 0x40, 0x18, 0x04, 0x10, 0x08,
  0x7e, 0xd5, 0x2a, 0x14, 0x24, 0x0a, 0x30, 0x04, 0x28, 0x08, 0x10,
};

const uint8_t F_CHEVRON = 1;
const uint8_t W_CHEVRON = 9;
const uint8_t PROGMEM chevron[F_CHEVRON * W_CHEVRON] =  // chevron
{
  0x18, 0x3c, 0x66, 0xc3, 0x99, 0x3c, 0x66, 0xc3, 0x81,
};

const uint8_t F_WALKER = 5;
const uint8_t W_WALKER = 7;
const uint8_t PROGMEM walker[F_WALKER * W_WALKER] =  // walking man
{
    0x00, 0x48, 0x77, 0x1f, 0x1c, 0x94, 0x68,
    0x00, 0x90, 0xee, 0x3e, 0x38, 0x28, 0xd0,
    0x00, 0x00, 0xae, 0xfe, 0x38, 0x28, 0x40,
    0x00, 0x00, 0x2e, 0xbe, 0xf8, 0x00, 0x00, 
    0x00, 0x10, 0x6e, 0x3e, 0xb8, 0xe8, 0x00,
};



struct bildlein                       // struct array for sprite effects
{
  const uint8_t *data;
  uint8_t width;
  uint8_t frames;
};

bildlein sprite[] =
{
  { walker, W_WALKER, F_WALKER },            //0
  { invader, W_INVADER, F_INVADER },         //1
  { chevron, W_CHEVRON, F_CHEVRON },         //3
  { heart, W_HEART, F_HEART },               //4
  { arrow1, W_ARROW1, F_ARROW1 },            //5
  { steamboat, W_STEAMBOAT, F_STEAMBOAT },   //6
  { fireball, W_FBALL, F_FBALL },            //7
  { rocket, W_ROCKET, F_ROCKET },            //8
  { roll2, W_ROLL2, F_ROLL2 },               //9
  { pacman2, W_PMAN2, F_PMAN2 },             //10
  { lines, W_LINES, F_LINES },               //11
  { roll1, W_ROLL1, F_ROLL1 },               //12
  { sailboat, W_SAILBOAT, F_SAILBOAT },      //13
  { arrow2, W_ARROW2, F_ARROW2 },            //14
  { wave, W_WAVE, F_WAVE },                  //15
  { pacman1, W_PMAN1, F_PMAN1 }              //16
};


//------------------------------------------------------------------------------------------------


 
////  Message Definitions  ////

#ifdef LOCAL_LANG

  const char msgWelcome[] =                                  "Willkommen";
                                                                                  /// messages only called by print functions (FPSTR not reqired) can be saved in flash to save RAM
  const char msgNews[] PROGMEM=                              "News von";
  const char msgCurrentWeather[] PROGMEM=                    "aktuell";
  const char msgForecast[] PROGMEM=                          "Vorhersage";
  const char msg24[] PROGMEM=                                "morgen";
  const char msg48[] PROGMEM=                                "übermorgen";
  const char msghour[]PROGMEM=                               "Uhr";
  const char msgFromTo[] PROGMEM=                            "bis";
  const char msgTemp[] PROGMEM=                              "Temperatur";
  const char msgHumidity[] PROGMEM=                          "Luftfeuchte";
  const char msgClouds[] PROGMEM=                            "Bewölkung";
  const char msgRain[] PROGMEM=                              "Niederschlag";
  const char msgSnow[] PROGMEM=                              "Schneemenge";
  const char msgWind[] PROGMEM=                              "Wind";
  const char msgNetwork[] PROGMEM=                           "Verbunden mit Netzwerk:";
  const char msgAs[] PROGMEM=                                "als";
  const char msgAP[] PROGMEM=                                "Betrieb als Access Point:";
  const char msgWith[] PROGMEM=                              "mit IP";
  const char msgNoNews[] PROGMEM=                            "News deaktiviert: Noch keine Nachrichten bezogen!";
  const char msgSendOwn[] PROGMEM=                           "Deine Nachricht auf dem Display an";
  const char msgOrdinalNumber[] PROGMEM=                     ".";
  const char msgSpotifyProb[] PROGMEM=                       "keine aktuelle Information verfügbar";
  const char msgSpotifyOK[] PROGMEM=                         "Es läuft gerade auf Spotify";
  const char msgSpotifyDeact[] PROGMEM=                      "Spotify ist deaktiviert";
  const char msgBy[] PROGMEM=                                "von";
  
#else
  
  const char msgWelcome[] =                                  "Welcome";
                                                                                   /// messages only called bei print functions (FPSTR not reqired) can be saved in flash to save RAM
  const char msgNews[] PROGMEM=                              "News from";
  const char msgCurrentWeather[] PROGMEM=                    "Current weather in";
  const char msgForecast[] PROGMEM=                          "Weather forecast for";
  const char msg24[] PROGMEM=                                "tomorrow";
  const char msg48[] PROGMEM=                                "48h";
  const char msghour[] PROGMEM=                              "h";
  const char msgFromTo[] PROGMEM=                            "-";
  const char msgTemp[] PROGMEM=                              "temperature";
  const char msgHumidity[] PROGMEM=                          "humidity";
  const char msgClouds[] PROGMEM=                            "clouds";
  const char msgRain[] PROGMEM=                              "rainfall";
  const char msgSnow[] PROGMEM=                              "snowfall";
  const char msgWind[] PROGMEM=                              "wind";
  const char msgNetwork[] PROGMEM=                           "Connected to network:";
  const char msgAs[] PROGMEM=                                "as";
  const char msgAP[]PROGMEM=                                 "Operation as Access Point:";
  const char msgWith[] PROGMEM=                              "with IP";
  const char msgNoNews[] PROGMEM=                            "News deactivated: No news loaded yet";
  const char msgSendOwn[] PROGMEM=                           "Send your own message to";
  const char msgOrdinalNumber[] PROGMEM=                     "";
  const char msgSpotifyProb[] PROGMEM=                       "no current information available";
  const char msgSpotifyOK[] PROGMEM=                         "Currently playing on Spotify";
  const char msgSpotifyDeact[] PROGMEM=                      "Spotify is deactivated";
  const char msgBy[] PROGMEM=                                "by";
  
#endif


////  FUNCTIONS  ////

////  WiFi ////


void wificonnect(){                                                   // read https://forum.arduino.cc/index.php?topic=652513 to understand how WiFi setup works on ESP

  #ifdef WIFI_IS_OFF_AT_BOOT                                          // enables WiFi connection on start up (restores original ESP8266 behaviour, which is disabled by default since ESP8266 core V 3.0)
    enableWiFiAtBootTime();                                           // see: https://github.com/esp8266/Arduino/blob/master/doc/esp8266wifi/generic-class.rst#persistent
  #endif
  
  uint8_t  wifi_retry=0;                                              // Counter solves ESP32-Bug with certain routers where connection can only be established every second time
  uint8_t  staticIP = 0;

  File f = SPIFFS.open("/IP_mode.txt", "r");                          // reads information saved on SPIFFS file if device shall connect with static IP
  String temp0 = f.readStringUntil('\n');
  temp0.trim();
  staticIP = temp0.toInt();
  String temp1 = f.readStringUntil('\n');
  temp1.trim();
  uint32_t IP_raw1 = temp1.toInt();
  IPAddress ip(IP_raw1);
  String temp2 = f.readStringUntil('\n');
  temp2.trim();
  uint32_t IP_raw2 = temp2.toInt();
  IPAddress gateway(IP_raw2);
  String temp3 = f.readStringUntil('\n');
  temp3.trim();
  uint32_t IP_raw3 = temp3.toInt();
  IPAddress subnet(IP_raw3);
  String temp4 = f.readStringUntil('\n');
  temp4.trim();
  uint32_t IP_raw4 = temp4.toInt();
  IPAddress dns(IP_raw4);
  f.close();

  #ifdef DEBUG
    Serial.println("IP Configuration loaded from SPIFFS:");
    Serial.print("Static IP enabled: ");
    Serial.println(staticIP);
    if (staticIP == 1) {
      Serial.print("IP: ");
      Serial.println(ip);
      Serial.print("Gateway: ");
      Serial.println(gateway);
      Serial.print("Subnet: ");
      Serial.println(subnet);
      Serial.print("DNS: ");
      Serial.println(dns);
    }
  #endif
  
  if (staticIP == 1) WiFi.config(ip, gateway, subnet, dns);             // sets static IP parameters
  WiFi.softAPdisconnect(true);                                          // closes AP mode on startup to avoid ESP working as AP during normal operation
  
#ifdef ESP32
  while (WiFi.status() != WL_CONNECTED && wifi_retry < 3) {
#else
  while (WiFi.waitForConnectResult() != WL_CONNECTED && wifi_retry < 3) {
#endif
        #ifdef DEBUG
          Serial.print("Not yet connected...retrying - Attempt No. ");
          Serial.println(wifi_retry+1);
        #endif
        WiFi.begin();
        delay(3000);
        wifi_retry++;
  } 

  if(wifi_retry >= 3) {
        #ifdef DEBUG
          Serial.println("no connection, starting AP");
          Serial.println("Notice on Display: Portal");
          Serial.println("... starting AP");
        #endif
        P.print("no Wifi --> AP");
        WiFiManager wifiManager;
      #ifndef SPOTIFY                                                         // this doesn't work with SPOTIFY defined on ESP32 !
        wifiManager.setAPStaticIPConfig(AP_IP, AP_IP, AP_Netmask);            // if #define SPOTIFY default ESP IP 192.168.4.1 is set
      #endif
        wifiManager.setConfigPortalTimeout(300);                              // timeout for configportal in seconds
        wifiManager.startConfigPortal(AP_NAME, AP_PW);

         if (wifiManager.getTimeoutState() == true) {                         // if config portal has timed out, ESP restarts(necessary after power blackout, when WiFi network needs some time to start up again)
          #ifdef DEBUG                                                        // (necessary after power blackout, when Home WiFi network needs some time to start up)
            Serial.println("Config Portal has timed out. Restarting...");
          #endif
          delay(500);
          ESP.restart();  
        }
        
        wifiManagerWasCalled = true;
             
        if (wifiManager.getStaticMode() == true) {                            // gets information from WiFiManager if connection has been made with static IP
            static_Mode_enabled = 1;                                          // and writes it to file on SPIFFS for future start ups. 
          }
        File f = SPIFFS.open("/IP_mode.txt", "w");
        f.printf("%i\n%i\n%i\n%i\n%i\n", static_Mode_enabled, uint32_t(WiFi.localIP()), uint32_t(WiFi.gatewayIP()), uint32_t(WiFi.subnetMask()), uint32_t(WiFi.dnsIP()));
        f.close();
        #ifdef DEBUG
          Serial.println("IP Configuration saved on SPIFFS:");
          Serial.print("Static IP enabled: ");
          Serial.println(static_Mode_enabled);
          Serial.print("IP: ");
          Serial.println(WiFi.localIP());
          Serial.print("Gateway: ");
          Serial.println(WiFi.gatewayIP());
          Serial.print("Subnet: ");
          Serial.println(WiFi.subnetMask());
          Serial.print("DNS: ");
          Serial.println(WiFi.dnsIP());
        #endif
  }
 
  if (WiFi.waitForConnectResult() == WL_CONNECTED){
        #ifdef DEBUG
          Serial.print("Connected to network \"");
          Serial.print(WiFi.SSID());
          Serial.print("\" with IP ");
          Serial.println(WiFi.localIP());
        #endif
        P.displayReset();
  }
}                                                                 // end of wificonnect()

#ifdef ESP32                                                           
 void wifiReconnect(){                                            // reconnection if WiFi is lost during operation - required only for ESP32
   uint8_t  wifi_retry=0;                                         // Number of reconnection attemps
   P.displayReset();
   P.print("lost WiFi - retrying..."); 
   while (WiFi.status() != WL_CONNECTED && wifi_retry < 10) {
      #ifdef DEBUG
        Serial.print("WiFi connection lost...retrying - Attempt No. ");
        Serial.println(wifi_retry+1);
      #endif
      WiFi.begin();
      delay(3000);
      wifi_retry++;
   } 

   if(wifi_retry >= 10) {
      #ifdef DEBUG
        Serial.println("reconnection impossible, restarting");
      #endif
      delay(1000);
      ESP.restart();
   }
 
   if (WiFi.waitForConnectResult() == WL_CONNECTED){
      #ifdef DEBUG
        Serial.println("successfully reconnected as");
        Serial.println(WiFi.localIP());
      #endif
      P.displayReset();
   }
 }
#endif


void checkIfAP(){                                                                      // Opens Access Point if captive portal is skipped via "Exit"
  if (WiFi.status() != WL_CONNECTED && wifiManagerWasCalled == true) {
          WiFi.mode(WIFI_AP);
          delay(500);
         // WiFi.softAPConfig(AP_IP, AP_IP, AP_Netmask);
          WiFi.softAP(AP_NAME, AP_PW);
        #ifdef DEBUG
          Serial.print("AP started with IP ");
          Serial.println(WiFi.softAPIP());
        #endif
          AP_established = true;
    }

//#ifndef ESP32
  if (WiFi.status() == WL_CONNECTED && wifiManagerWasCalled == true) {                  // ?? restart necessary only for ESP8266 due to memory limitation when calling getWeatherData()
    P.print("WiFi ok");
    delay(2000);
   #ifdef DEBUG
    Serial.println("Wifi ok, restarting device");
   #endif
    P.print("restarting");
    delay(500);
    ESP.restart();
    }
//#endif
}



//// UTF8 CONVERSION ////                                    // Note: Convert only one time, otherwise characters will disappear!

uint8_t utf8Ascii(uint8_t ascii) {                           // See https://forum.arduino.cc/index.php?topic=171056.msg4457906#msg4457906
                                                             // and http://playground.arduino.cc/Main/Utf8ascii
  static uint8_t cPrev;
  static uint8_t cPrePrev;
  uint8_t c = '\0';
  
  if (ascii < 0x7f || ascii == degreeC_ascii || ascii == degreeF_ascii) {
    cPrev = '\0';                                           // last character
    cPrePrev = '\0';                                        // penultimate character
    c = ascii;
  } else {
    switch (cPrev) {
      case 0xC2: c = ascii;  break;
      case 0xC3: c = ascii | 0xC0;  break;
      }
     switch (cPrePrev) {
      
      case 0xE2: if (cPrev==0x82 && ascii==0xAC) c = 128;  // EURO SYMBOL          // 128 -> number in defined font table
                 if (cPrev==0x80 && ascii==0xA6) c = 133;  // horizontal ellipsis
                 if (cPrev==0x80 && ascii==0x93) c = 150;  // en dash
                 if (cPrev==0x80 && ascii==0x9E) c = 132;  // DOUBLE LOW-9 QUOTATION MARK
                 if (cPrev==0x80 && ascii==0x9C) c = 147;  // LEFT DOUBLE QUOTATION MARK
                 if (cPrev==0x80 && ascii==0x9D) c = 148;  // RIGHT DOUBLE QUOTATION MARK
                 if (cPrev==0x80 && ascii==0x98) c = 145;  // LEFT SINGLE QUOTATION MARK
                 if (cPrev==0x80 && ascii==0x99) c = 146;  // RIGHT SINGLE QUOTATION MARK
                 
                 break; 
      }

    cPrePrev = cPrev;                                      // save penultimate character
    cPrev = ascii;                                         // save last character
  }
  return(c);
}


void utf8AsciiConvert(char* src, char*des)                 // converts array form source array "src" to destination array "des"
{       int k=0;
        char c;
        for (int i=0; src[i]; i++){
             c = utf8Ascii(src[i]);
             if (c!='\0') //if (c!=0)
             des[k++]=c;
        }
        des[k]='\0';  //des[k]=0;
}



//// TIME AND DATE FUNCTIONS ////


void getTimeFromServer(){
  uint8_t  time_retry=0;                                         // Counter retry counts time server
 #ifdef ESP32 
  configTzTime(timezone, ntpServer);                             // adjust your local time zone with variable timezone
 #endif 
  struct tm initial;                                             // temp struct for checking if year==1970 (no received time information means year is 1970)
  initial.tm_year=70;
  
  while(initial.tm_year == 70 && time_retry < 15){                 
  #ifdef ESP32                                                   // get time from NTP server (ESP32)
   getLocalTime(&initial);                                       
  #else                                                          // get time from NTP server (ESP8266)
   if (esp8266::coreVersionNumeric() >= 20700000){               
      configTime(timezone, ntpServer); 
   } else {                                                      // compatibility with ESP8266 Arduino Core Versions < 2.7.0
      setenv("TZ", timezone , 1);
      configTime(0, 0, ntpServer);
   }                            
  #endif 
   delay(500);
   if (sntp_getreachability(0) != 0){                            // if sntp_getreachability(0) == 0 -> ntp server call failed
    time_t now = time(&now);
    localtime_r(&now, &initial);
   }
  #ifdef DEBUG
   Serial.print("Time Server connection attempt: ");
   Serial.println(time_retry + 1);
   Serial.print("current year: ");
   Serial.println(1900 + initial.tm_year);
  #endif
   time_retry++;
  }

  if (time_retry >=15){
    #ifdef DEBUG
      Serial.println("Connection to time server failed");
    #endif  
  } else {
    time_t now = time(&now);
    localtime_r(&now, &tm);
    if (enableTime==1){
      strftime (timeshow, sizeof(timeshow), "%H:%M", &tm);
    #ifdef DEBUG
      Serial.print("Successfully requested current time from server: ");
      Serial.println(timeshow); 
    #endif
    }
  }
}


void makeDate() {
  char buf1[20]; 
  char buf2[20];
  char buf3[20];
  char buf4[20];
  uint8_t weekday;

  time_t now = time(&now);
  localtime_r(&now, &tm);

  if (enableDate == 1){
    #ifdef SHORTDATE
      strftime (buf1, sizeof(buf1), "%e", &tm);                                                           // see: http://www.cplusplus.com/reference/ctime/strftime/
      snprintf (buf2, sizeof(buf2), "%s", months[tm.tm_mon]);                                             // see: http://www.willemer.de/informatik/cpp/timelib.htm
      strftime (buf3, sizeof(buf3), "%Y", &tm);
      snprintf (dateshow, sizeof(dateshow), "%s%s %s %s", buf1,msgOrdinalNumber, buf2, buf3);                // Example: 1 Mar 2020 / 1. Mrz 2020
    #else
      snprintf (buf1, sizeof(buf1), "%s", days[tm.tm_wday]);
      strftime (buf2, sizeof(buf2), "%e", &tm);
      snprintf (buf3, sizeof(buf3), "%s", monthsXL[tm.tm_mon]);
      strftime (buf4, sizeof(buf4), "%Y", &tm);
      snprintf (dateshow, sizeof(dateshow), "%s %s%s %s %s", buf1, buf2, msgOrdinalNumber, buf3, buf4);     // Example: Sunday 1 March 2020 / Sonntag 1. März 2020
      utf8AsciiConvert(dateshow, dateshow);        // conversion necessary for "Jänner" and "März"
    #endif
  }

  weekday = tm.tm_wday;
  switch (weekday) {
    case 0: 
      snprintf (dayAfterTomorrow, sizeof(dayAfterTomorrow), "%s" ,days[2]);                               // if today is Sunday, day after tomorrow is Tuesday
      break;
    case 1: 
      snprintf (dayAfterTomorrow, sizeof(dayAfterTomorrow), "%s" ,days[3]);
      break;
    case 2: 
      snprintf (dayAfterTomorrow, sizeof(dayAfterTomorrow), "%s" ,days[4]);
      break;
    case 3: 
      snprintf (dayAfterTomorrow, sizeof(dayAfterTomorrow), "%s" ,days[5]);
      break;
    case 4: 
      snprintf (dayAfterTomorrow, sizeof(dayAfterTomorrow), "%s" ,days[6]);
      break;
    case 5: 
      snprintf (dayAfterTomorrow, sizeof(dayAfterTomorrow), "%s" ,days[0]);
      break;
    case 6: 
      snprintf (dayAfterTomorrow, sizeof(dayAfterTomorrow), "%s" ,days[1]);
      break;
  }
    
#ifdef DEBUG
  Serial.print("Date: ");
  Serial.println(dateshow);
#endif
}


void displayTime() {                                                                // standard case: if messages other than time are activated
  static time_t lastminute = 0;
  time_t now = time(&now);
  localtime_r(&now, &tm);
  if (tm.tm_min != lastminute) {
    lastminute = tm.tm_min;
    if (enableTime==1){
      strftime (timeshow, sizeof(timeshow), "%H:%M", &tm);
     #ifdef DEBUG 
      Serial.print("current time: ");
      Serial.println(timeshow);
     #endif
    }
  }
    
  if (tm.tm_hour == 0 && tm.tm_min == 0 && tm.tm_sec == 0) makeDate();              // at 0:00 make new date
}


void displayOnlyTime() {                                                            // special case: if only time message is activated -> hh:mm:ss
  P.setTextAlignment(PA_CENTER);
  static time_t lastsecond = 0;
  time_t now = time(&now);
  localtime_r(&now, &tm);
  if (tm.tm_sec != lastsecond) {
    lastsecond = tm.tm_sec;
    if (enableTime==1){
      strftime (timeshow, sizeof(timeshow), "%H:%M:%S", &tm);
      P.setFont(numeric7Seg);
      P.displayReset();
      P.print(timeshow);
    } else {
      P.displayReset();
      P.displayClear();                                                             // display shows nothing
    }
   }
  if (millis() - previoustimecall > (60* 10000)){                                   // gets time from server every 60 minutes
    getTimeFromServer();                                         
    previoustimecall=millis();
  }    
  if (tm.tm_hour == 0 && tm.tm_min == 0 && tm.tm_sec == 0) makeDate();
}


////  WEATHER FUNCTIONS  ////  

String getWindDirection (int degrees) {
  int sector = ((degrees + 11) / 22.5 - 1);
  return windDirection[sector];
}


void getWeatherData() {                                              //gets weather data for all data and saves it into table.wetcur                      
   static bool currentWeath = true;
   String units;
   uint8_t tempUnit;
   if (imperial == false){
      units = "metric";
      tempUnit = degreeC_ascii;
   } else {
      units = "imperial";
      tempUnit = degreeF_ascii;
   }
   WiFiClient client;
   HTTPClient http;
  
   #ifdef DEBUG
    Serial.println("Calling Weather");
   #endif
    
  for (uint8_t w=0; w<numberofcities*2; w++){
      String cityID = ownCityIDs[WeatherCityLoop];
      String url;
  
      if (currentWeath){                                                                
        url = "http://api.openweathermap.org/data/2.5/weather?id=" + cityID + "&units=" + units + "&lang=" + weatherLanguage + "&APPID=" + WeatherApiKey;
      } else {
        url ="http://api.openweathermap.org/data/2.5/forecast?id=" + cityID + "&units=" + units + "&cnt=16&lang=" + weatherLanguage + "&APPID=" + WeatherApiKey; 
      }
  
      if(http.begin(client,url)){    
        int httpCode = http.sendRequest("GET");
        #ifdef DEBUG  
          Serial.println(httpCode);
        #endif    
        if(httpCode == 200) {
          
          DynamicJsonDocument doc(12000);
          DeserializationError error = deserializeJson(doc, http.getStream());
          
          if (error) {
            #ifdef DEBUG 
              Serial.println("deserializeJson() failed: ");
              Serial.println(error.c_str());
              #endif
            //File f = SPIFFS.open("/logfile.txt", "a");              /// logfile DEBUG -> If you want to use this, remember the file size enlarges with every call !!!
            //f.printf("%s\n%s\n", timeshow, "ERROR WEATHER");        /// logfile DEBUG
            //f.close();                                              /// logfile DEBUG
            return;
          }
            
          if (currentWeath) {

            JsonObject weather_0 = doc["weather"][0];
            const char* current_description = weather_0["description"];           // "clear sky"

            JsonObject main = doc["main"];                  // EXAMPLE:
            float current_temp = main["temp"];              // -0.44
            uint8_t current_humidity = main["humidity"];    // 92
            float main_temp_min = main["temp_min"];         // -2.78
            float main_temp_max = main["temp_max"];         // 2.22
            float wind_speed = doc["wind"]["speed"];        // 0.5
            int wind_deg = doc["wind"]["deg"];              // 260
            float rain_3h = doc["rain"]["3h"];              // 0.19
            float snow_3h = 0;
            snow_3h = doc["snow"]["3h"];
             
            uint8_t current_clouds = doc["clouds"]["all"];  // 25
            const char* cityname = doc["name"];             // Vienna

            char msgNiederschlag_3h[20];
            if (snow_3h == 0){
              snprintf(msgNiederschlag_3h, sizeof(msgNiederschlag_3h),"%s %.1f mm", msgRain, rain_3h);
            } else {
              snprintf(msgNiederschlag_3h, sizeof(msgNiederschlag_3h),"%s %.1f cm", msgSnow, snow_3h);
            }

            snprintf(table[w].wetcur, sizeof(table[w].wetcur), "%s %s: %s, %s %.1f %c  %s %d%%  %s %d%%  %s  %s %.0f m/s %s",
            msgCurrentWeather, cityname, current_description, msgTemp, current_temp, tempUnit, msgHumidity, current_humidity, msgClouds, current_clouds,
            msgNiederschlag_3h, msgWind, wind_speed, getWindDirection(wind_deg).c_str());

            utf8AsciiConvert(table[w].wetcur, table[w].wetcur);
      
          } else {
       
            JsonArray list = doc["list"];

            JsonObject list_7 = list[7];
            const char* time7 = list_7["dt_txt"];

            JsonObject list_7_main = list_7["main"];                       // EXAMPLE:
            float temp24 = list_7_main["temp"];                            // -0.64
            float tempmin24 = list_7_main["temp_min"];                     // 3.45
            float tempmax24 = list_7_main["temp_max"];                     // 6.25
            uint8_t humidity24 = list_7_main["humidity"];

            JsonObject list_7_weather_7 = list_7["weather"][0];
            const char* description24 = list_7_weather_7["description"];   // Bedeckt
            uint8_t clouds24 = list_7["clouds"]["all"];
            float windspeed24 = list_7["wind"]["speed"];                   // 3.1
            int winddeg24 = list_7["wind"]["deg"];                         // 202
            float rain24 = list_7["rain"]["3h"];
            float snow24 = 0;
            snow24 = list_7["snow"]["3h"];

            JsonObject list_15 = list[15];
            const char* time15 = list_15["dt_txt"];

            JsonObject list_15_main = list_15["main"];
            float temp48 = list_15_main["temp"];
            float tempmin48 = list_15_main["temp_min"];
            float tempmax48 = list_15_main["temp_max"];
            uint8_t humidity48 = list_15_main["humidity"];

            JsonObject list_15_weather_15 = list_15["weather"][0];
            const char* description48 = list_15_weather_15["description"];
            uint8_t clouds48 = list_15["clouds"]["all"];
            float windspeed48 = list_15["wind"]["speed"];
            int winddeg48 = list_15["wind"]["deg"];
            float rain48 = list_15["rain"]["3h"];
            float snow48 = 0;
            snow48 = list_15["snow"]["3h"];

            char msgNiederschlag_24h[20];
            if (snow24 == 0){
              snprintf(msgNiederschlag_24h, sizeof(msgNiederschlag_24h),"%s %.1f mm", msgRain, rain24);
            } else {
              snprintf(msgNiederschlag_24h, sizeof(msgNiederschlag_24h),"%s %.1f cm", msgSnow, snow24);
            }

            char msgNiederschlag_48h[20];
            if (snow48 == 0){
              snprintf(msgNiederschlag_48h, sizeof(msgNiederschlag_48h),"%s %.1f mm", msgRain, rain48);
            } else {
              snprintf(msgNiederschlag_48h, sizeof(msgNiederschlag_48h),"%s %.1f cm", msgSnow, snow48);
            }
            

            JsonObject city = doc["city"];
            const char* cityname = city["name"];                              // "Vienna"
      
            char time24[6] = {""};
            char time48[6] = {""};
      
            snprintf(time24, sizeof(time24),"%c%c:%c%c", time7[11], time7[12], time7[14], time7[15]);
            snprintf(time48, sizeof(time48),"%c%c:%c%c", time15[11], time15[12], time15[14], time15[15]);
      
            char forecast24[200] = {""};
            char forecast48[200] = {""};

            snprintf(forecast24, sizeof(forecast24), "%s %s, %s %s %s: %s, %s %.1f %c  %s %d%%  %s %d%%  %s  %s %.0f m/s %s",
            msgForecast, cityname, msg24, time24, msghour, description24, msgTemp, temp24, tempUnit, msgHumidity, humidity24, msgClouds, clouds24,
            msgNiederschlag_24h, msgWind, windspeed24, getWindDirection(winddeg24).c_str());
            utf8AsciiConvert(forecast24, forecast24);

            snprintf(forecast48, sizeof(forecast48), "%s %s, %s %s %s: %s, %s %.1f %c  %s %d%%  %s %d%%  %s  %s %.0f m/s %s",
            msgForecast, cityname, dayAfterTomorrow, time48, msghour, description48, msgTemp, temp48, tempUnit, msgHumidity, humidity48, msgClouds, clouds48,
            msgNiederschlag_48h, msgWind, windspeed48, getWindDirection(winddeg48).c_str());
            utf8AsciiConvert(forecast48, forecast48);
            snprintf(table[w].wetcur, sizeof(table[w].wetcur), "%s    %s", forecast24, forecast48);
          }
          weathEverLoaded = true;
        } else {
          #ifdef DEBUG 
            Serial.printf("http error: %i\n", httpCode);
          #endif
        }
      WeatherCityLoop = (++WeatherCityLoop) % ARRAY_SIZE(ownCityIDs);
      if (WeatherCityLoop == 0)                                             // after a loop of all cities, swicth from current weather to forecast
        currentWeath = !currentWeath;
      #ifdef DEBUG
        Serial.println(table[w].wetcur);                                  
      #endif
      //File f = SPIFFS.open("/logfile.txt", "a");                /// logfile DEBUG -> If you want to use this, remember the file size enlarges with every call !!!
      //f.printf("%s\n%s\n", timeshow, table[w].wetcur);          /// logfile DEBUG
      //f.close();                                                /// logfile DEBUG
    } else {
      #ifdef DEBUG 
        Serial.println("Unable to connect to weather API");
      #endif
    }
    http.end();
  }
}




////  NEWS FUNCTIONS  ////

void getNewsData() {
  WiFiClient client;
  HTTPClient http;

  #ifdef DEBUG 
    Serial.println("Calling News");
  #endif
 
  for (uint8_t w=0; w<newssources; w++){                               //loop for number of news sources
    String url;
    uint8_t newsmultiplier = numofarticles * w;                        //newsmultiplier necessary to determine right position in table[].newscur
  
    url = "http://newsapi.org/v2/" + ownNewsSources[w] + "&pageSize=3&page=1&apiKey=" + NewsApiKey;

    if(http.begin(client,url)){    

      int httpCode = http.sendRequest("GET");
      #ifdef DEBUG  
        Serial.println(httpCode);
      #endif
      if(httpCode == 200) {

        DynamicJsonDocument doc(5000);
        DeserializationError error = deserializeJson(doc, http.getStream());
        
        if (error) {
          #ifdef DEBUG 
            Serial.println("deserializeJson() failed: ");
            Serial.println(error.c_str());
          #endif
          //File f = SPIFFS.open("/logfile.txt", "a");              /// logfile DEBUG -> If you want to use this, remember the file size enlarges with every call !!!
          //f.printf("%s\n%s\n", timeshow, "ERROR NEWS");           /// logfile DEBUG
          //f.close();                                              /// logfile DEBUG
          return;
        }
        
        JsonArray articles = doc["articles"];

        JsonObject articles_0 = articles[0];                                                        // EXAMPLE:
        bool news0_ok = false;
        const char* name0;
        const char* title0;
        const char* description0;
        const char* url0;
        if (articles_0) {                                                                           //see: https://arduinojson.org/v6/api/jsonobject/containskey/                      
          news0_ok = true;
          name0 = articles_0["source"]["name"];                                                     // "Spiegel Online"
          title0 = articles_0["title"];                                                             // "Wahlkampf in Großbritannien: Der Brexit-Vagabund"
          if (articles_0["description"]) {description0 = articles_0["description"];}     //const char* description0 = articles_0["description"] -> "Der Londoner Tory-Abgeordnete Greg Hands war 2016 gegen den Brexit, mittlerweile wirbt er dafür.... 
            else {description0 = " - keine Detailinfo verfügbar - ";}
          url0 = articles_0["url"];                                                                 // "http://www.spiegel.de/politik/ausland/brexit-im-wahlkampf-mit-tory-politiker-greg-hands-a-1300489.html"
        }
        
        JsonObject articles_1 = articles[1];
        bool news1_ok = false;
        const char* name1;
        const char* title1;
        const char* description1;
        const char* url1;
        if (articles_1) {
          news1_ok = true;
          name1 = articles_1["source"]["name"];
          title1 = articles_1["title"];
          if (articles_1["description"]) {description1 = articles_1["description"];} 
            else {description1 = " - keine Detailinfo verfügbar - ";}
          url1 = articles_1["url"];
        }

        JsonObject articles_2 = articles[2];
        bool news2_ok = false;
        const char* name2;
        const char* title2;
        const char* description2;
        const char* url2;
        if (articles_2) {
          news2_ok = true;
          name2 = articles_2["source"]["name"];
          title2 = articles_2["title"];
          if (articles_2["description"]) {description2 = articles_2["description"];} 
            else {description2 = " - keine Detailinfo verfügbar - ";}
          url2 = articles_2["url"];
        }

        if (enableNewsXL==1){
          if (news0_ok) snprintf(table[newsmultiplier].newscur, sizeof(table[newsmultiplier].newscur), "%s %s: %s: %s", msgNews, name0, title0, description0);
            else snprintf(table[newsmultiplier].newscur, sizeof(table[newsmultiplier].newscur), "check news source: %s: no info", ownNewsSources[w].c_str());
          if (news1_ok) snprintf(table[newsmultiplier+1].newscur, sizeof(table[newsmultiplier+1].newscur), "%s %s: %s: %s", msgNews, name1, title1, description1);
            else snprintf(table[newsmultiplier+1].newscur, sizeof(table[newsmultiplier+1].newscur), "check news source: %s: no info", ownNewsSources[w].c_str());
          if (news2_ok)snprintf(table[newsmultiplier+2].newscur, sizeof(table[newsmultiplier+2].newscur), "%s %s: %s: %s", msgNews, name2, title2, description2);
            else snprintf(table[newsmultiplier+2].newscur, sizeof(table[newsmultiplier+2].newscur), "check news source: %s: no info", ownNewsSources[w].c_str());
        } else {
          if (news0_ok) snprintf(table[newsmultiplier].newscur, sizeof(table[newsmultiplier].newscur), "%s %s: %s", msgNews, name0, title0);
            else snprintf(table[newsmultiplier].newscur, sizeof(table[newsmultiplier].newscur), "check news source: %s", ownNewsSources[w].c_str());
          if (news1_ok) snprintf(table[newsmultiplier+1].newscur, sizeof(table[newsmultiplier+1].newscur), "%s %s: %s", msgNews, name1, title1);
            else snprintf(table[newsmultiplier+1].newscur, sizeof(table[newsmultiplier+1].newscur), "check news source: %s", ownNewsSources[w].c_str());
          if (news2_ok) snprintf(table[newsmultiplier+2].newscur, sizeof(table[newsmultiplier+2].newscur), "%s %s: %s", msgNews, name2, title2);
            else snprintf(table[newsmultiplier+2].newscur, sizeof(table[newsmultiplier+2].newscur), "check news source: %s", ownNewsSources[w].c_str());
        }
        if (news0_ok) snprintf(table[newsmultiplier].newslink, sizeof(table[newsmultiplier].newslink), "%s", url0);
        if (news1_ok)snprintf(table[newsmultiplier+1].newslink, sizeof(table[newsmultiplier+1].newslink), "%s", url1);
        if (news2_ok)snprintf(table[newsmultiplier+2].newslink, sizeof(table[newsmultiplier+2].newslink), "%s", url2);
        
        newsEverLoaded = true;
      } else {
        #ifdef DEBUG 
          Serial.printf("http error: %i\n", httpCode);
        #endif 
      }
      #ifdef DEBUG
        Serial.println(table[newsmultiplier].newscur);
        Serial.println(table[newsmultiplier+1].newscur);
        Serial.println(table[newsmultiplier+2].newscur);
        Serial.println(table[newsmultiplier].newslink);
        Serial.println(table[newsmultiplier+1].newslink);
        Serial.println(table[newsmultiplier+2].newslink);
      #endif
      //File f = SPIFFS.open("/logfile.txt", "a");                                                                                                  /// logfile DEBUG -> If you want to use this, remember the file size enlarges with every call !!!
      //f.printf("%s\n%s\n%s\n%s\n", timeshow, table[newsmultiplier].newscur, table[newsmultiplier+1].newscur, table[newsmultiplier+2].newscur);    /// logfile DEBUG
      //f.close();                                                                                                                                  /// logfile DEBUG
   } else {
      #ifdef DEBUG 
        Serial.println("Unable to connect to news API");
      #endif 
   }
   http.end();   
  }      
}



#ifdef SPOTIFY
/// SPOTIFY FUNCTIONS ///

void spotiauth() {                        // initiates Spotify authentication process -> for further information see: https://developer.spotify.com/documentation/general/guides/authorization-guide/#authorization-code-flow 
  #ifdef DEBUG
    Serial.println("First Step: Calling Spotify for one time authentication token...");
  #endif 
  String firstContact = "https://accounts.spotify.com/de/authorize/?client_id=" + String(clientID) + "&response_type=code&redirect_uri=" + String(redirectUri) + "&scope=user-read-currently-playing";
  server.sendHeader("Location", firstContact);
  server.send(302, "text/plain", "ok");    
}


void spoticallback() {                               // handles complete Spotify authentication process
  if(!server.hasArg("code")){
    server.send(500, "text/plain", "Server call failed!");
    return;
  }
  String oneTimeCode;
  oneTimeCode = server.arg("code");                  // one time token sent by spotify                  
  #ifdef DEBUG 
    Serial.printf("Second Step: Received one time authentication token: %s  --> Calling for persistent refresh token...\n", oneTimeCode.c_str());
  #endif
  bool authSuccess = false;
  HTTPClient http;
  if(http.begin("https://accounts.spotify.com/api/token", digicert_CA_pem)) {
    
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    String completeAuth = "grant_type=authorization_code&code=" + oneTimeCode + "&redirect_uri=" + String(redirectUri) + "&client_id=" + String(clientID) + "&client_secret=" + String(clientSecret);
    int httpCode =  http.POST(completeAuth);
    #ifdef DEBUG
      Serial.println(httpCode);
    #endif

    if(httpCode == 200) {                     
      DynamicJsonDocument doc(500);                                                   // Json document received from Spotify
      DeserializationError error = deserializeJson(doc, http.getStream());
      if (error) {
        #ifdef DEBUG
          Serial.println("deserializeJson() failed: ");
          Serial.println(error.c_str());
        #endif 
      return;
      }
      const char* initialAccesstoken = doc["access_token"];                          // access token valid until expire time elapses
      const char* initialRefreshtoken = doc["refresh_token"];                        // persistent refresh token for all future Spotify api calls, will be saved on SPIFFS 
      accesstoken = "Bearer " + String(initialAccesstoken);
      refreshtoken = initialRefreshtoken;
      expiretime = doc["expires_in"];
      auth = "Basic " + base64::encode(String(clientID) + ":" + String(clientSecret));
      authSuccess = true;
      } else {
      #ifdef DEBUG 
        Serial.printf("http error: %i\n", httpCode);
      #endif
      } 
    } else {
      #ifdef DEBUG 
        Serial.println("Unable to connect to Spotify");
      #endif
    }
  http.end();
  
  if (authSuccess == true){
     #ifdef DEBUG
       Serial.println("Spotify Authentication Process completed:");
       Serial.print("Refreshtoken: ");
       Serial.println(refreshtoken);
       Serial.print("Accesstoken: ");
       Serial.println(accesstoken);
       Serial.print("expires in ");
       Serial.print(expiretime);
       Serial.println(" seconds.");
     #endif
     
     File f = SPIFFS.open("/spotifyAuth.txt", "w");                         // writes persistent refreshtoken and auth credentials to file -> will be read on startup
     f.printf("%s\n%s\n", auth.c_str(), refreshtoken.c_str());
     f.close();
  
      #ifdef DEBUG
        Serial.println("Spotify credentials saved to file");
      #endif
      //parseSpotify();                                                     // you can uncomment this for debugging purposes
     
      char successUri[40];
      snprintf(successUri, sizeof(successUri), "http://%d.%d.%d.%d/spotAuthOK.html", WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3]);
      server.sendHeader("Location",String(successUri));                     // redirets to html-page with success message
      server.send(303);
   } else {
      char FailUri[40];
      snprintf(FailUri, sizeof(FailUri), "http://%d.%d.%d.%d/spotAuthFail.html", WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3]);
      server.sendHeader("Location",String(FailUri));                        // redirets to html-page with fail message
      server.send(303);
   }
}


void refreshSpotify(){                                                      // demands and receives a new access token (on startup or when expiretime has elapsed)
  uint8_t refresh_retry = 0;
  bool refresh_success = false;
  
  while(refresh_retry < 5 && refresh_success == false){  
    #ifdef DEBUG 
      Serial.println("Calling Spotify for accesstoken");
    #endif 
    HTTPClient http;
  
    if(http.begin("https://accounts.spotify.com/api/token", digicert_CA_pem)){
      http.addHeader("Content-Type", "application/x-www-form-urlencoded"); 
      http.addHeader("Authorization", auth);
  
      String url = "grant_type=refresh_token&refresh_token=" + refreshtoken;
      int httpCode =  http.POST(url);
      #ifdef DEBUG 
        Serial.println(httpCode);
      #endif

      if(httpCode == 200) {
        DynamicJsonDocument doc(400);
        DeserializationError error = deserializeJson(doc, http.getStream());
        if (error) {
          #ifdef DEBUG
            Serial.println("deserializeJson() failed: ");
            Serial.println(error.c_str());
          #endif
          refresh_retry++;
          return;
        }
        const char* newtoken = doc["access_token"];
        expiretime = doc["expires_in"];
        #ifdef DEBUG 
          Serial.print("New accesstoken: ");
          Serial.print(newtoken);
          Serial.print(" expires in ");
          Serial.print(expiretime);
          Serial.println(" seconds.");
        #endif 
        accesstoken = "Bearer " + String(newtoken);
        #ifdef DEBUG
          Serial.println(accesstoken);
        #endif
        refresh_success = true;
        //parseSpotify();                                   // you can uncomment this for debugging purposes
      } else {
        #ifdef DEBUG 
          Serial.printf("http error: %i\n", httpCode);
        #endif
        refresh_retry++;
      }
    } else {
      #ifdef DEBUG 
        Serial.println("Unable to connect to Spotify");
      #endif
      refresh_retry++;
    }
    http.end();
  }
  
  if(refresh_retry >= 5) {
    enableSpotify = 0;
    #ifdef DEBUG 
      Serial.println("Call for accesstoken failed! Spotify message disabled.");
    #endif
  }
}



void parseSpotify(){                                  // calls Spotify api with accesstoken and receives Json document with data about currently playing song                                    
  HTTPClient http;
  #ifdef DEBUG 
    Serial.println("Calling Spotify for currently playing");
  #endif

  if(http.begin("https://api.spotify.com/v1/me/player/currently-playing", digicert_CA_pem)){
    http.addHeader("Accept", "application/json");
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Authorization", accesstoken);
    int httpCode = http.sendRequest("GET");
    #ifdef DEBUG 
      Serial.println(httpCode);
    #endif 
    
    if(httpCode == 200) {
      DynamicJsonDocument doc(12000);
      DeserializationError error = deserializeJson(doc, http.getStream());
      if (error) {
        snprintf(showspotify, sizeof(showspotify), "");
        snprintf(spotify, sizeof(spotify), "%s (parserr)", msgSpotifyProb);        // information on spotify.html if parsing error occurs
        snprintf(spotifycover, sizeof(spotifycover), "/Spotprob.jpg");
        #ifdef DEBUG
          Serial.println("deserializeJson() failed: ");
          Serial.println(error.c_str());
        #endif
        return;
      }
      JsonObject item = doc["item"];
      JsonObject item_album = item["album"];
      JsonArray item_album_images = item_album["images"];
      JsonObject item_album_images_0 = item_album_images[0];
      const char* coverurl = item_album_images_0["url"];
      const char* item_name = item["name"];
      JsonObject item_artists_0 = item["artists"][0];
      const char* artist = item_artists_0["name"];
  
      #ifdef DEBUG
        Serial.println("Spotify Response:");
        Serial.println(coverurl);
        Serial.println(artist);
        Serial.println(item_name);
      #endif 
  
      snprintf(spotify, sizeof(spotify), "%s<br><span class=\"von\">%s</span><br>%s", item_name, msgBy, artist);             // information shown on spotify.html
      snprintf(showspotify, sizeof(showspotify), "%s: \"%s\" %s %s", msgSpotifyOK, item_name, msgBy, artist);                        // information shown on display
      utf8AsciiConvert(showspotify, showspotify);
      strcpy(spotifycover,coverurl);
  
      #ifdef DEBUG 
        Serial.println(showspotify);
      #endif
  
    } else {
      snprintf(showspotify, sizeof(showspotify), "");
      snprintf(spotify, sizeof(spotify), "%s (!=200)", msgSpotifyProb);         // information on spotify.html if no data available
      snprintf(spotifycover, sizeof(spotifycover), "/Spotprob.jpg");
      #ifdef DEBUG 
        Serial.printf("http error: %i\n", httpCode);
      #endif 
    }
  } else {
    snprintf(showspotify, sizeof(showspotify), "");
    snprintf(spotify, sizeof(spotify), "%s (nocon)", msgSpotifyProb);          // information on spotify.html if no connection
    snprintf(spotifycover, sizeof(spotifycover), "/Spotprob.jpg");
    #ifdef DEBUG 
      Serial.println("Unable to connect to Spotify");
    #endif
  }
http.end();
}



void loadSpotifyAuth() {                                                      // loads Spotify authentication data from file on SPIFFS on startup
    File f = SPIFFS.open("/spotifyAuth.txt", "r");
    String temp = f.readStringUntil('\n');
    temp.trim();
    auth = temp;
    P.setIntensity(intens);
    String temp2 = f.readStringUntil('\n');
    temp2.trim();
    refreshtoken = temp2;
    #ifdef DEBUG
      Serial.println("Spotify Data loaded from file: ");
      Serial.print("Auth: ");
      Serial.println(auth);
      Serial.print("Refreshtoken: ");
      Serial.println(refreshtoken);
    #endif
    f.close();
}

#endif                                    // end #ifdef SPOTIFY




 
/// SPIFFS AND WEBSERVER FUNCTIONS ///


void html_authentify (String& site) {                                                              // verifies authentication for calls via webserver
  if (!server.authenticate(www_username, www_password)) {
      return server.requestAuthentication();
    } 
  if (server.hasArg("delete")) {
        SPIFFS.remove(server.arg("delete"));                                                      // deletes file
        server.sendHeader("Location","/spiffs.html");
        server.send(303);
  } else {
        File f = SPIFFS.open(site, "r"); server.streamFile(f, contentType(site)); f.close(); 
  }
}

void html_authentify_ota() {
  if (!server.authenticate(www_username, www_password)) {
      return server.requestAuthentication();
    }
 #ifdef OTA
    if (server.hasArg("delete")) {
          SPIFFS.remove(server.arg("delete"));                                                          
          server.sendHeader("Location","/spiffs.html");
          server.send(303);
    } else {
        File f = SPIFFS.open("/ota.html", "r"); server.streamFile(f, "text/html"); f.close(); 
    }
 #else
    if (server.hasArg("delete")) {
          SPIFFS.remove(server.arg("delete"));                                                          
          server.sendHeader("Location","/spiffs.html");
          server.send(303);
    } else {
          File f = SPIFFS.open("/noota.html", "r"); server.streamFile(f, "text/html"); f.close(); 
    }
 #endif
}

void send404() {
   if (server.hasArg("delete")) {
          SPIFFS.remove(server.arg("delete"));                                                          
          server.sendHeader("Location","/spiffs.html");
          server.send(303);
    } else {
          File f = SPIFFS.open("/404.html", "r"); server.streamFile(f, "text/html"); f.close(); 
    }
}


const char Helper[] PROGMEM = R"(<form method="POST" action="/upload" enctype="multipart/form-data">              //enables uploading a file before spiffs.html is present on SPIFFS
     <input type="file" name="upload"><input type="submit" value="Upload"></form>Lade die spiffs.html hoch.)";


const String &contentType(String& filename) {                                                   // identifies content type of file
  if (filename.endsWith(".htm") || filename.endsWith(".html")) filename = "text/html";
  else if (filename.endsWith(".css")) filename = "text/css";
  else if (filename.endsWith(".js")) filename = "application/javascript";
  else if (filename.endsWith(".json")) filename = "application/json";
  else if (filename.endsWith(".png")) filename = "image/png";
  else if (filename.endsWith(".gif")) filename = "image/gif";
  else if (filename.endsWith(".jpg")) filename = "image/jpeg";
  else if (filename.endsWith(".ico")) filename = "image/x-icon";
  else if (filename.endsWith(".xml")) filename = "text/xml";
  else if (filename.endsWith(".pdf")) filename = "application/x-pdf";
  else if (filename.endsWith(".zip")) filename = "application/x-zip";
  else if (filename.endsWith(".gz")) filename = "application/x-gzip";
  else filename = "text/plain";
  return filename;
}

bool handleFile(String&& path) {
  if (server.hasArg("delete")) {
    SPIFFS.remove(server.arg("delete"));                                                          // deletes file
    server.sendHeader("Location","/spiffs.html");
    server.send(303); 
    return true;
  }
  if (!SPIFFS.exists("/spiffs.html"))server.send(200, "text/html", Helper);                       // enables uploading a file before spiffs.html is present on SPIFFS:
  if (path.endsWith("/")) path += "index.html";
  if (path == "/admin.html" || path == "/config.html" || path == "/spiffs.html"){
    return SPIFFS.exists(path) ? ({html_authentify(path); true;}) : false;
  } else {
    return SPIFFS.exists(path) ? ({File f = SPIFFS.open(path, "r"); server.streamFile(f, contentType(path)); f.close(); true;}) : false;
  }
}


const String formatBytes(size_t const& bytes) {                                                   // transforms file size into common format
  return (bytes < 1024) ? String(bytes) + " Byte" : (bytes < (1024 * 1024)) ? String(bytes / 1024.0) + " KB" : String(bytes / 1024.0 / 1024.0) + " MB";
}

void formatSpiffs() {                                                                             // formats SPIFFS Memory
  SPIFFS.format();
  server.sendHeader("Location","/spiffs.html");
  server.send(303); 
}

#ifdef ESP32
void handleList() {                                                                               // sends a list of all files on SPIFFS to requesting html page
  
  File root = SPIFFS.open("/");
  String temp = "[";
  File file = root.openNextFile();
  while (file) {
    if (temp != "[") temp += ",";
    #ifdef ESP_ARDUINO_VERSION                                                                                          // for ESP32 core Version >=2.0.0
      temp += R"({"name":")" + String(file.name()) + R"(","size":")" + formatBytes(file.size()) + R"("})";
    #else
      temp += R"({"name":")" + String(file.name()).substring(1) + R"(","size":")" + formatBytes(file.size()) + R"("})";
    #endif
    file = root.openNextFile();
  }
  temp += R"(,{"usedBytes":")" + formatBytes(SPIFFS.usedBytes() * 1.05) + R"(",)" +              // calculates used memory + additional 5% for safety reasons
          R"("totalBytes":")" + formatBytes(SPIFFS.totalBytes()) + R"(","freeBytes":")" +        // shows total memory
          (SPIFFS.totalBytes() - (SPIFFS.usedBytes() * 1.05)) + R"("}])";                        // calculates free memory + additional 5% for safety reasons
  server.send(200, "application/json", temp);
}


void handleUpload() {                                                                             // loads file from computer and saves it to SPIFFS
  static File fsUploadFile;                                                                       // keeps current upload
  HTTPUpload& upload = server.upload();
  if (upload.status == UPLOAD_FILE_START) {
    if (upload.filename.length() > 30) {
      upload.filename = upload.filename.substring(upload.filename.length() - 30, upload.filename.length());           // shortens file name to a length of 30 characters
    }
   #ifdef DEBUG 
    DEBUG("FileUpload Name: " + upload.filename);
   #endif 
    fsUploadFile = SPIFFS.open("/" + server.urlDecode(upload.filename), "w");
  } else if (upload.status == UPLOAD_FILE_WRITE) {
   #ifdef DEBUG  
    DEBUG("FileUpload Data: " + (String)upload.currentSize);
   #endif 
    if (fsUploadFile)
      fsUploadFile.write(upload.buf, upload.currentSize);
  } else if (upload.status == UPLOAD_FILE_END) {
    if (fsUploadFile)
      fsUploadFile.close();
   #ifdef DEBUG    
    DEBUG("FileUpload Size: " + (String)upload.totalSize);
   #endif 
    server.sendHeader("Location","/spiffs.html");
    server.send(303); 
  }
}

#else

void handleList() {                                                                         // sends a list of all files on SPIFFS to requesting html page
  FSInfo fs_info;  SPIFFS.info(fs_info);                                                    // fills up FSInfo structure with information about file system
  Dir dir = SPIFFS.openDir("/");                                                            // lists all files saved on SPIFFS
  String temp = "[";
  while (dir.next()) {
    if (temp != "[") temp += ',';
    temp += "{\"name\":\"" + dir.fileName().substring(1) + "\",\"size\":\"" + formatBytes(dir.fileSize()) + "\"}";
  }
  temp += ",{\"usedBytes\":\"" + formatBytes(fs_info.usedBytes * 1.05) + "\"," +             // calculates used memory + additional 5% for safety reasons
          "\"totalBytes\":\"" + formatBytes(fs_info.totalBytes) + "\",\"freeBytes\":\"" +    // shows total memory
          (fs_info.totalBytes - (fs_info.usedBytes * 1.05)) + "\"}]";                        // calculates free memory + additional 5% for safety reasons
  server.send(200, "application/json", temp);
}

void handleUpload() {                                                                         // loads file from computer and saves it to SPIFFS
  static File fsUploadFile;                                                                   // keeps current upload
  HTTPUpload& upload = server.upload();
  if (upload.status == UPLOAD_FILE_START) {
    if (upload.filename.length() > 30) {
      upload.filename = upload.filename.substring(upload.filename.length() - 30, upload.filename.length());        // shortens file name to a length of 30 characters
    }
   #ifdef DEBUG
    Serial.printf("handleFileUpload Name: /%s\n", upload.filename.c_str());
   #endif
    fsUploadFile = SPIFFS.open("/" + server.urlDecode(upload.filename), "w");
  } else if (upload.status == UPLOAD_FILE_WRITE) {
   #ifdef DEBUG 
    Serial.printf("handleFileUpload Data: %u\n", upload.currentSize);
   #endif
    if (fsUploadFile)
      fsUploadFile.write(upload.buf, upload.currentSize);
  } else if (upload.status == UPLOAD_FILE_END) {
    if (fsUploadFile)
      fsUploadFile.close();
   #ifdef DEBUG
    Serial.printf("handleFileUpload Size: %u\n", upload.totalSize);
   #endif
    server.sendHeader("Location","/spiffs.html");
    server.send(303); 
  }
}
 
#endif  //endif ESP32




void showSimple() {                                                                                 // Sends ownmessage to requesting html page                       
  String onkele = ownmessage;
  String temp = "{\"No1\":\""+ onkele + "\" , \"No2\":\""+ enableOwn + "\"  }";                     // String temp is sent to requesting html page
  server.send(200, "application/json", temp); 
  #ifdef DEBUG
    Serial.print("Ownmessage: ");
    Serial.print(onkele);
    Serial.println(" sent succesfully");
  #endif
}

 
void composeNews(uint8_t startPos) {                                                     // composes Strings and sends news to requesting html page
                                                                                         // is executed for a number of six news only due to ESP8266 memory limitation
  String bbc[6];                                                                         // startPos defines start position in table[].newscur      
  String source[2]; 
  uint8_t t;
  uint8_t helpcounter = 0;
  for (t=startPos; t<(startPos+6); t++){
    String newsline = table[t].newscur;
     if (t==startPos) {
      source[0]=newsline.substring(0,newsline.indexOf(':'));
     }
    if (t==startPos+3) {
      source[1]=newsline.substring(0,newsline.indexOf(':'));
     }
    String postnews = newsline.substring(newsline.indexOf(':')+1);                                  // cuts News from presenter,eg: "News von Spiegel-Online:"
    postnews.replace("\"", "\\\"");                                                                 // keeps quotation marks during sending to html page
    String postlink = table[t].newslink;
    bbc[helpcounter] = "\"News"+ String(t) +"\":\""+ postnews + "\" , \"NewsURL" + String(t) +"\":\"" + postlink + "\"";
    #ifdef DEBUG
      Serial.print("String: ");
      Serial.print(bbc[helpcounter]);
      Serial.println(" composed successfully");
    #endif
    helpcounter++;
}
 String temp = "{" + bbc[0] + "," + bbc[1] + "," + bbc[2] + "," + bbc[3] + "," + bbc[4] + "," + bbc[5] + ", \"S1\":\""+ source[0] + "\" , \"S2\":\""+ source[1] + "\"}";
 server.send(200, "application/json", temp);
 #ifdef DEBUG
  Serial.print("String: ");
  Serial.print(temp);
  Serial.println(" sent successfully");
 #endif 
}

void sendNews(){
composeNews(0);
}

void sendNews2(){
composeNews(6);
}

void showAdvanced (){                                                                               // Sends all messages and enabling values to requesting html page
  String dlf[4];
  for (int t=0; t<4; t++){
   
    String onkele = messages[t];
    dlf[t] = "\"Enable"+ String(t) +"\":\""+ enablemessage[t]  + "\", \"In"+ String(t) +"\":\""+ catalogTEMP[t].effectinTEMP + "\" , \"Text" + String(t) +"\":\"" + onkele + "\", \"Out" + String(t) +"\":\"" + catalogTEMP[t].effectoutTEMP + "\" , \"Align" + String(t) +"\":\"" + catalogTEMP[t].justTEMP + "\" , \"Speed" + String(t) +"\":\"" + catalogTEMP[t].speedTEMP + "\", \"Pause" + String(t) +"\":\"" + catalogTEMP[t].pauseTEMP + "\"";
    #ifdef DEBUG
      Serial.print("String: ");
      Serial.print(dlf[t]);
      Serial.println(" composed successfully");
    #endif
  }
 String temp = "{" + dlf[0] + "," + dlf[1] + "," + dlf[2] + "," + dlf[3] + ",\"EnableTime\":\"" + enableTime + "\" ,\"EnableDate\":\"" + enableDate + "\", \"EnableWeath\":\"" + enableWeath + "\", \"EnableNews1\":\"" + enableNews1 + "\", \"EnableSpot\":\"" + enableSpotify + "\", \"EnableNews2\":\"" + enableNews2 + "\" ,\"EnableOwn\":\"" + enableOwn + "\" ,\"EnableOwn\":\"" + enableOwn + "\"}";
 
 server.send(200, "application/json", temp);
 #ifdef DEBUG
  Serial.print("String: ");
  Serial.print(temp);
  Serial.println(" sent successfully");
 #endif
}

void allOffCheck(){                                                                           // checks enable state of messages and activates mode "Only showing Time"
  if (enableDate==0 && enablemessage[0]==0 && enableWeath==0 && enableNews1==0 && enablemessage[1]==0 && enablemessage[2]==0 && enablemessage[3]==0
      && enableSpotify==0 && enableNews2==0 && enableOwn==0) { 
      timeOnly = true;
  } else {
      timeOnly = false;
  }
      }

void enableOrDisable(uint8_t loc, char* msg, uint8_t ena, uint8_t specialcase) {             // main function for defining values of all messages EXCEPT...
  if (ena==1){                                                                               // ...the custom messages of array messages[] which are handles via admin html page
      if (specialcase==2){                  //special case timeshow effect and pause
        catalog[loc].effectin = 1;
        catalog[loc].effectout = 1;
        catalog[loc].just = 1;
        catalog[loc].speed = 4;
        catalog[loc].pause = 6;
        catalog[loc].delay = 1;
      } 
      if (specialcase==3){                  //special case dateshow effect and pause
       #ifdef SHORTDATE
        catalog[loc].effectin = 2;
        catalog[loc].effectout = 2;
        catalog[loc].just = 1;
        catalog[loc].speed = 4;
        catalog[loc].pause = 6;
        catalog[loc].delay = 1;
       #else
        catalog[loc].effectin = 3;
        catalog[loc].effectout = 3;
        catalog[loc].just = 1;
        catalog[loc].speed = globScrollSpeed;
        catalog[loc].pause = 0;
        catalog[loc].delay = 1;
       #endif
      }
      if (specialcase==0){                // standard for all other messages
        catalog[loc].effectin = 3;
        catalog[loc].effectout = 3;
        catalog[loc].just = 1;
        catalog[loc].speed = globScrollSpeed;
        catalog[loc].pause = 0;
        catalog[loc].delay = 1;
      }
      if(specialcase==1){                 // special case showspotify delay
        catalog[loc].effectin = 3;
        catalog[loc].effectout = 3;
        catalog[loc].just = 1;
        catalog[loc].speed = globScrollSpeed;
        catalog[loc].pause = 0;
        catalog[loc].delay = 0;
      }
  } else {                                 // standard if messages deactivated
      snprintf(msg, sizeof(msg), "");
      catalog[loc].effectin = 0;
      catalog[loc].effectout = 26;
      catalog[loc].just = 1;
      catalog[loc].speed = 0;
      catalog[loc].pause = 0;
      catalog[loc].delay = 0;
      }
}


void handleSimple() {                                                                               // processes ownmessage received from html page
  if (server.hasArg("myText")){
   snprintf(ownmessage, sizeof(ownmessage), "%s",server.arg("myText").c_str());
   #ifdef DEBUG
    Serial.print(ownmessage);
    Serial.println(" received and processed successfully");
   #endif
   server.send(200, "text/plain", "");   //sends nothing to invisible iframe on webpage
   }
}


void handleAdvanced (){                                                  // processes all custom messages and enabling data received from html page
  if (server.hasArg("MyEnable0")){
    for (int t=0; t<4; t++){
      String zero = "MyEnable" + String(t);
      String one = "MyIn" + String(t); 
      String two = "MyText" + String(t);
      String three = "MyOut" + String(t);
      String four = "MyAlign" + String(t);
      String five = "MySpeed" + String(t);
      String six = "MyPause" + String(t);
      

      enablemessage[t] = server.arg(zero).toInt();

      #ifdef DEBUG
        Serial.print("Custom Message ");
        Serial.print(t+1);
        Serial.print(" enabled: ");
        Serial.print(enablemessage[t]);
        Serial.println("  -> 1 == yes, 0 == no");
      #endif

      snprintf(messages[t], sizeof(messages[t]), "%s",server.arg(two).c_str());

      catalogTEMP[t].effectinTEMP = server.arg(one).toInt();

      catalogTEMP[t].effectoutTEMP = server.arg(three).toInt();

      catalogTEMP[t].justTEMP = server.arg(four).toInt();

      catalogTEMP[t].speedTEMP = server.arg(five).toInt();

      catalogTEMP[t].pauseTEMP =  server.arg(six).toInt();

      catalogTEMP[t].delayTEMP = 1;

    #ifdef DEBUG
      Serial.print("String ");
      Serial.print(messages[t]);
      Serial.println(" received successfully");
    #endif    
  }

  enableTime = server.arg("MyEnableTime").toInt();
  enableOrDisable(locationTime, timeshow, enableTime, 2);
  if (enableTime==1){
      time_t now = time(&now);
      localtime_r(&now, &tm);
      strftime (timeshow, sizeof(timeshow), "%H:%M", &tm);
  }

  enableDate = server.arg("MyEnableDate").toInt();
  enableOrDisable(locationDate, dateshow, enableDate, 3);
  if (enableDate==1) makeDate();

  enableWeath = server.arg("MyEnableWeath").toInt();
  enableOrDisable(locationWeath, showweather, enableWeath, 0);

  enableNews1 = server.arg("MyEnableNews1").toInt();
  enableOrDisable(locationNews[0], shownews, enableNews1, 0);
  
  enableSpotify = server.arg("MyEnableSpot").toInt();
   if (enableSpotify==1){
      #ifdef SPOTIFY
        enableOrDisable(locationSpot,showspotify, enableSpotify,1);
      #else
        enableSpotify=0;
      #endif
   } else {
        snprintf(spotify, sizeof(spotify), msgSpotifyDeact);
        snprintf(spotifycover, sizeof(spotifycover), "/noSpot.jpg");
        enableOrDisable(locationSpot,showspotify, enableSpotify,1);
      #ifdef SPOTIFY
       expiretime = 1;
      #endif
   }
  
  enableNews2 = server.arg("MyEnableNews2").toInt();
  enableOrDisable(locationNews[1], shownews2, enableNews2, 0);

  enableOwn = server.arg("MyEnableOwn").toInt();
  enableOrDisable(locationOwn, showownmessage, enableOwn, 0);

 if (enableWeath==1 && weathEverLoaded == false) getWeatherData();
 
 bool newsActiveCheck = false;
 if (enableNews1 == 1 || enableNews2 == 1) newsActiveCheck = true;
 if (newsActiveCheck == true && newsEverLoaded == false) getNewsData();

 if (enableOwn==0) changeflag = true;
 if (enableOwn==1 && changeflag == true ) {
     if (AP_established == false){
            snprintf(ownmessage, sizeof(ownmessage), "--> %s %d.%d.%d.%d <--", msgSendOwn, WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3]);
     } else {
            snprintf(ownmessage, sizeof(ownmessage), "--> %s %d.%d.%d.%d <--", msgSendOwn,  WiFi.softAPIP()[0], WiFi.softAPIP()[1], WiFi.softAPIP()[2], WiFi.softAPIP()[3]);
     }
 }
 if (enableOwn==1) changeflag = false; 


 allOffCheck();
  
 server.send(200, "text/plain", "");
  }
}


void saveAllMessages (){                                                                          // saves all custom messages and enabling data to file on SPIFFS
  String path;
  if(AP_established == false){                                                                    // normal operation with WiFi connection
      path = "/messages.txt";
  } else {                                                                                        // operation as access point
      path = "/AP_only.txt";                                                                      
  }
  File f = SPIFFS.open(path, "w"); 
  for (int t=0; t<4; t++){
    f.printf("%i{%i{%s{%i{%i{%i{%i\n", enablemessage[t], catalogTEMP[t].effectinTEMP, messages[t], catalogTEMP[t].effectoutTEMP, catalogTEMP[t].justTEMP, catalogTEMP[t].speedTEMP, catalogTEMP[t].pauseTEMP);
    }
  f.printf("%i\n%i\n%i\n%i\n%i\n%i\n%i\n", enableTime, enableDate, enableWeath, enableNews1, enableSpotify, enableNews2, enableOwn);
  f.close();
  server.send(200, "text/plain", "");
 #ifdef DEBUG
  Serial.println("Messages saved successfully!");
 #endif
}


void loadAllMessages (uint8_t defmessage, bool avoidOnStartup) {                         // loads and processes all messages and enabling data from file on SPIFFS                            
  String path;                                                                           // see files messages.txt and default.txt
  if (defmessage==0){
    path = "/messages.txt";
    }
  if (defmessage==1){
    path = "/default.txt";                                          
    }
  if (defmessage==2){
    path = "/AP_only.txt";                                          
    }
  if (defmessage==3){
    path = "/AP_only_default.txt";                                          
    }
  File f = SPIFFS.open(path , "r");
  for (int t=0; t<4; t++){
    String line = f.readStringUntil('\n');
    line.trim();
    #ifdef DEBUG
      Serial.println("Loaded from file: ");
      Serial.println(line.substring(0));
      Serial.println("processing String:");
    #endif
   
   int index0;
   int index1;                            // split String "result" and format the new Strings for parsing:
   int index2;                            // vgl. https://www.theamplituhedron.com/articles/How-to-split-strings-in-Arduino-IDE-to-glean-information-as-substrings/
   int index3;
   int index4;
   int index5;
   int index6;
   

   index0 = line.indexOf("{");   
   String test0 = line.substring(0,index0);
   #ifdef DEBUG
    Serial.println(test0);
   #endif
   index1 = line.indexOf("{", index0+1);   
   String test1 = line.substring(index0+1,index1);
   #ifdef DEBUG
    Serial.println(test1);
   #endif
   index2 = line.indexOf("{", index1+1);
   String test2 = line.substring(index1+1,index2);
   #ifdef DEBUG
    Serial.println(test2);
   #endif
   index3 = line.indexOf("{", index2+1);
   String test3 = line.substring(index2+1,index3);
   #ifdef DEBUG
    Serial.println(test3);
   #endif
   index4 = line.indexOf("{", index3+1);
   String test4 = line.substring(index3+1,index4);
   #ifdef DEBUG
    Serial.println(test4);
   #endif
   index5 = line.indexOf("{", index4+1);
   String test5 = line.substring(index4+1,index5);
   #ifdef DEBUG
    Serial.println(test5);
   #endif
   index6 = line.indexOf("{", index5+1);
   String test6 = line.substring(index5+1,index6);
   #ifdef DEBUG
    Serial.println(test6);
   #endif
        
   enablemessage[t] = test0.toInt();

   snprintf(messages[t], sizeof(messages[t]), "%s", test2.c_str());

   catalogTEMP[t].effectinTEMP = test1.toInt();
   catalogTEMP[t].effectoutTEMP = test3.toInt();
   catalogTEMP[t].justTEMP = test4.toInt();
   catalogTEMP[t].speedTEMP = test5.toInt();
   catalogTEMP[t].pauseTEMP = test6.toInt();
   catalogTEMP[t].delayTEMP = 1;
  }

  String lineTime = f.readStringUntil('\n');
    lineTime.trim();
    enableTime = lineTime.toInt();
    enableOrDisable(locationTime, timeshow, enableTime, 2);
    if (enableTime==1){
      time_t now = time(&now);
      localtime_r(&now, &tm);
      strftime (timeshow, sizeof(timeshow), "%H:%M", &tm);
  }

  String lineDate = f.readStringUntil('\n');
    lineDate.trim();
    enableDate = lineDate.toInt();
    enableOrDisable(locationDate, dateshow, enableDate, 3);
    if (enableDate==1) makeDate();

 String lineWeath = f.readStringUntil('\n');
    lineWeath.trim();
    enableWeath = lineWeath.toInt();
    enableOrDisable(locationWeath, showweather, enableWeath, 0);

 String lineNews1 = f.readStringUntil('\n');
    lineNews1.trim();
    enableNews1 = lineNews1.toInt();
    enableOrDisable(locationNews[0], shownews, enableNews1, 0);
    
 String lineSpot = f.readStringUntil('\n');
    lineSpot.trim();
    enableSpotify = lineSpot.toInt();
if (enableSpotify==1){
      #ifdef SPOTIFY
        enableOrDisable(locationSpot,showspotify, enableSpotify,1);
      #else
        enableSpotify=0;
      #endif
   } else {
        snprintf(spotify, sizeof(spotify), msgSpotifyDeact);
        snprintf(spotifycover, sizeof(spotifycover), "/noSpot.jpg");
        enableOrDisable(locationSpot,showspotify, enableSpotify,1);
      #ifdef SPOTIFY
       expiretime = 1;
      #endif
   }

 String lineNews2 = f.readStringUntil('\n');
    lineNews2.trim();
    enableNews2 = lineNews2.toInt();
    enableOrDisable(locationNews[1], shownews2, enableNews2, 0);

 String lineOwn = f.readStringUntil('\n');
    lineOwn.trim();
    enableOwn = lineOwn.toInt();
    enableOrDisable(locationOwn, showownmessage, enableOwn, 0);

 if (avoidOnStartup==false){
     if (enableWeath==1 && weathEverLoaded == false) getWeatherData();
      }
 
 bool newsActiveCheck = false;
 if (enableNews1 == 1 || enableNews2 == 1) newsActiveCheck = true;
 if (newsActiveCheck == true && newsEverLoaded == false) getNewsData();

 if (enableOwn==0) changeflag = true;
 if (enableOwn==1 && changeflag == true ) {
     if (AP_established == false){
            snprintf(ownmessage, sizeof(ownmessage), "--> %s %d.%d.%d.%d <--", msgSendOwn, WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3]);
     } else {
            snprintf(ownmessage, sizeof(ownmessage), "--> %s %d.%d.%d.%d <--", msgSendOwn,  WiFi.softAPIP()[0], WiFi.softAPIP()[1], WiFi.softAPIP()[2], WiFi.softAPIP()[3]);
     }
 }
 if (enableOwn==1) changeflag = false;

 allOffCheck();

 f.close();
}

void loadAll() {                                                                         // load SAVED custom messages and enabling data from file on SPIFFS
  if(AP_established == false){
    loadAllMessages(0, false);
  } else {
    loadAllMessages(2, false);
  }
}

void loadDefault() {                                                                     // load DEFAULT custom messages and enabling data from file on SPIFFS
  if(AP_established == false){
    loadAllMessages(1, false);
  } else {
    loadAllMessages(3, false);
  }
}

void showConfig() {                                                                      // sends all configuration data to requesting html page
  int minutes = millis()/60000;
  String Network_Name;
  String Network_IP;
  if(AP_established == false){
    Network_Name = WiFi.SSID().c_str();
    Network_IP = WiFi.localIP().toString();
  } else {
    String MyAP = AP_NAME;
    Network_Name = "AP " + MyAP;
    Network_IP = WiFi.softAPIP().toString();    
  }
  String temp = "{\"Intens\":\""+ String(intens) + "\", \"Minutes\":\""+ String(minutes) + "\", \"SSID\":\""+ Network_Name + "\", \"IP\":\""+ Network_IP + "\", \"EnableXL\":\""+ enableNewsXL + "\", \"Speed\":\""+ globScrollSpeed + "\" }";
  server.send(200, "application/json", temp);
  #ifdef DEBUG
    Serial.println("current configuration: ");
    Serial.print("Intensity: ");
    Serial.println(intens);
    Serial.print("Global Scroll Speed (internal value): ");
    Serial.println(globScrollSpeed);
    Serial.print("Enable NewsXL: ");
    Serial.println(enableNewsXL);
  #endif
}

void handleConfigIntens() {                                                             // receives and processes config data (intensity) sent from config html page
  if (server.hasArg("newIntens")){
   intens = server.arg("newIntens").toInt();
   P.setIntensity(intens);
   #ifdef DEBUG
    Serial.println("Config received: ");
    Serial.print("Intensity: ");
    Serial.println(intens);
   #endif
   server.send(200, "text/plain", "");
   }
}

void handleConfigSpeed() {                                                             // receives and processes config data (global scroll speed) sent from config html page
  if (server.hasArg("newSpeed")){
   globScrollSpeed = server.arg("newSpeed").toInt();
   setGlobalScrollSpeed();
   #ifdef DEBUG
    Serial.println("Config received: ");
    Serial.print("Global Scroll Speed (internal value): ");
    Serial.println(globScrollSpeed);
   #endif
   server.send(200, "text/plain", "");
   }
}

void saveConfig (){                                                                   // saves all configuration data to file on SPIFFS
  File f = SPIFFS.open("/config.txt", "w");
  f.printf("%i\n%i\n%i\n", intens, enableNewsXL, globScrollSpeed);
  f.close();
  server.send(200, "text/plain", "");
  #ifdef DEBUG
    Serial.println("Config saved to file: ");
    Serial.print("Intensity: ");
    Serial.println(intens);
    Serial.print("Enable NewsXL: ");
    Serial.println(enableNewsXL);
    Serial.print("Global Scroll Speed (internal value): ");
    Serial.println(globScrollSpeed);
   #endif
}

void loadConfig () {                                                               // loads all configuration data from file on SPIFFS
    File f = SPIFFS.open("/config.txt", "r");
    String temp = f.readStringUntil('\n');
    temp.trim();
    intens = temp.toInt();
    P.setIntensity(intens);
    String temp2 = f.readStringUntil('\n');
    temp2.trim();
    enableNewsXL = temp2.toInt();
    String temp3 = f.readStringUntil('\n');
    temp3.trim();
    globScrollSpeed = temp3.toInt();
    setGlobalScrollSpeed();
    #ifdef DEBUG
      Serial.println("Config loaded from file: ");
      Serial.print("Intensity: ");
      Serial.println(intens);
      Serial.print("Enable NewsXL: ");
      Serial.println(enableNewsXL);
      Serial.print("Global Scroll Speed (internal value): ");
      Serial.println(globScrollSpeed);
    #endif
    f.close();
}

void configNews(){                                                                     // receives and processes enabling News XL Data and refreshes news data
  if (server.hasArg("MyEnableXL")){
    enableNewsXL = server.arg("MyEnableXL").toInt();
    #ifdef DEBUG
      Serial.print("Enable News XL: ");
      Serial.println(enableNewsXL);
    #endif
    server.send(200, "text/plain", "");
  }
  refreshNews();
}

void setGlobalScrollSpeed(){                                                          // sets speed for all scrolling messages (date, weather, news, ownmessage, spotify)
catalog[locationDate].speed = globScrollSpeed;
catalog[locationWeath].speed = globScrollSpeed;                                                              
catalog[locationNews[0]].speed = globScrollSpeed;                 
catalog[locationNews[1]].speed = globScrollSpeed;                            
catalog[locationSpot].speed = globScrollSpeed;                           
catalog[locationOwn].speed = globScrollSpeed;
}                      


void resetDevice(){               // restarts device
  P.setFont(tobFont);             // Parola standard font set, necessary if MAIN DISPLAY ROUTINE is at position locationTime
  P.print("restarting...");
  delay(1000);
  ESP.restart();
}

void refreshNews(){               // refreshes news data
  P.setFont(tobFont);
  P.print("getting news");
  getNewsData();
  delay(500);
  P.displayReset();
  P.displayAnimate();
  P.displayClear();
}


void refreshWeather(){            // refreshes weather data
  P.setFont(tobFont);
  P.print("getting weath");
  getWeatherData();
  delay(500);
  P.displayReset();
  P.displayAnimate();
  P.displayClear();   
}

void clearCredentials() {         // erases WiFi credentials persistently saved in flash memory and restarts device -> WifiManager captive portal is shown on follwing start up
  #ifdef DEBUG
   Serial.println("erasing WiFi credentials");
  #endif
  static_Mode_enabled = 0;
  File f = SPIFFS.open("/IP_mode.txt", "w");      // resets IP config file on SPIFFS to default values
  f.printf("%i\n%i\n%i\n%i\n%i\n", static_Mode_enabled, uint32_t(WiFi.localIP()), uint32_t(WiFi.gatewayIP()), uint32_t(WiFi.subnetMask()), uint32_t(WiFi.dnsIP()));
  f.close();
  #ifdef ESP32
   WiFi.disconnect(true,true);
  #else
   WiFi.disconnect(true);
  #endif
  #ifdef DEBUG
   Serial.println("restarting");
  #endif
  P.print("erasing WiFi...");
  delay(5000);
  ESP.restart();
}

void showCover() {                                 // sends Spotify information and cover url to requesting html page
 String music = spotify;
 music.replace("\"","'");                          // replace " by ' to prevent Json error in html
 String pic = spotifycover;
 String temp = "{\"Info\":\""+ music + "\", \"Url\":\""+ pic + "\" }";
 server.send(200, "application/json", temp);
 #ifdef DEBUG
  Serial.println("String: ");
  Serial.print(temp);
  Serial.println(" sent successfully");
 #endif 
}

String sketchName() {                                               // shortens file name for sendSketchName()
  char file[sizeof(__FILE__)] = __FILE__;
  char * pch;
  char * svr;
  pch = strtok (file,"\\");
  while (pch != NULL){ 
    svr = pch;
    pch = strtok (NULL, "\\");
  }
  return svr;
}  

void sendSketchName(){
  String temp = "{\"Name\":\""+ sketchName() + "\"  }";             // sends name of current sketch to requesting web page
  server.send(200, "application/json", temp); 
  #ifdef DEBUG
    Serial.print("Current sketch name: ");
    Serial.print(sketchName());
    Serial.println(" sent successfully");
  #endif
}



void listener() {                                                  // handles all server requests from html pages
  server.on("/showSimple", showSimple);
  server.on("/showNews", sendNews);
  server.on("/showNews2", sendNews2);
  server.on("/showAdvanced", showAdvanced);
  server.on("/postFormSimple", handleSimple);
  server.on("/postFormAdv", handleAdvanced);
  server.on("/saveAll", saveAllMessages);
  server.on("/loadAll", loadAll);
  server.on("/loadDefault", loadDefault);
  server.on("/showConfig", showConfig);
  server.on("/postConfigIntens", handleConfigIntens);
  server.on("/postConfigSpeed", handleConfigSpeed);
  server.on("/saveConfig", saveConfig);
  server.on("/reset", resetDevice);
  server.on("/news", configNews);
  server.on("/weather", refreshWeather);
  server.on("/ex", clearCredentials);
  server.on("/cover", showCover);
  server.on("/ota.html", html_authentify_ota);
  server.on("/sketchName", sendSketchName);
  
  server.onNotFound([]() {
            if (!handleFile(server.urlDecode(server.uri())))
              send404();
             });
                   
  server.on("/json", handleList);
  server.on("/format", formatSpiffs);
  server.on("/upload", HTTP_POST, []() {}, handleUpload);


 
 #ifdef OTA                   /// OVER THE AIR UPDATE VIA WEBBROWSER (OTA)

 #ifdef ESP32                                           // see: https://lastminuteengineers.com/esp32-ota-web-updater-arduino-ide/
  server.on("/ota", HTTP_POST, [=]() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", (Update.hasError()) ? OTAfailMsg : OTAsuccessMsg);
    delay(1500);
    ESP.restart();
    }, []() {
    P.setFont(tobFont);                             
    P.print("OTA running...");
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
     #ifdef DEBUG
      Serial.printf("Update: %s\n", upload.filename.c_str());
     #endif
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
      #ifdef DEBUG
        Update.printError(Serial);
      #endif
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
       #ifdef DEBUG
        Update.printError(Serial);
       #endif
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { //true to set the size to the current progress
       #ifdef DEBUG
        Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
       #endif
      } else {
       #ifdef DEBUG
        Update.printError(Serial);
       #endif
      }
    }
  });

#else                       // else if ESP8266               see: https://github.com/esp8266/Arduino/blob/master/libraries/ESP8266WebServer/examples/WebUpdate/WebUpdate.ino
      server.on("/ota", HTTP_POST, [=]() {
      server.sendHeader("Connection", "close");
      server.send(200, "text/html", (Update.hasError()) ? OTAfailMsg : OTAsuccessMsg);
      delay(1000);
      ESP.restart();
      }, []() {
      P.setFont(tobFont);
      P.print("OTA running...");
      HTTPUpload& upload = server.upload();
      if (upload.status == UPLOAD_FILE_START) {
        WiFiUDP::stopAll();
       #ifdef DEBUG
        Serial.printf("Update: %s\n", upload.filename.c_str());
       #endif
        uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
        if (!Update.begin(maxSketchSpace)) { //start with max available size
         #ifdef DEBUG
          Update.printError(Serial);
         #endif
        }
      } else if (upload.status == UPLOAD_FILE_WRITE) {
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
         #ifdef DEBUG
          Update.printError(Serial);
         #endif
        }
      } else if (upload.status == UPLOAD_FILE_END) {
        if (Update.end(true)) { //true to set the size to the current progress
         #ifdef DEBUG
          Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
         #endif
        } else {
         #ifdef DEBUG
          Update.printError(Serial);
         #endif
        }
      }
      yield();
    });
 #endif                 //endif ESP32/ESP8266
 #endif                 //endif OTA


 #ifdef SPOTIFY
  server.on("/spotiauth", spotiauth);                               // starts Spotify authentication process
  server.on("/callback", spoticallback);
 #endif
 
}                      // end of function listener()

 

void setup(void){
  P.begin();                                 // Parola begin
  #ifdef DEBUG
    Serial.begin(57600);
  #endif
 
  SPIFFS.begin();
 
  loadConfig();                              // loads configuration saved in SPIFFS 
 
  P.setTextAlignment(PA_CENTER);
  P.print("starting up...");
  P.displayReset();
  
  wificonnect();                             // connects to WiFi or establishes access point

  checkIfAP();                               // sets up AP if WifiManager config portal has been skipped

  P.print("getting data");

  if(AP_established == false){                      // normal operation with wifi connection
      getTimeFromServer();                          // calls time server for current time
      makeDate();                                   // forced call to enable writing into of dayAfterTomorrow
    #ifdef ESP32  
      loadAllMessages(0, false);                    // loads messages and messages config saved in SPIFFS
    #else
       #ifdef DEBUG                                 // forces loading weather data on startup due to possible memory problems with this function during runtime on ESP8266
          getWeatherData();                         
          loadAllMessages(0, true);
       #else
          loadAllMessages(0, false);  
       #endif               
    #endif
  } else {                                          // operation as AP
      loadAllMessages(2, false);                    
  }
  
  if (enableNews1==0 && enableNews2==0) snprintf(table[0].newscur, sizeof(table[0].newscur), "%s", msgNoNews);     // message shown in news.html when no news loaded

#ifdef SPOTIFY
  loadSpotifyAuth();
  if (enableSpotify == 1){
    #ifdef DEBUG
      Serial.println("Free heap: ");
      Serial.println(ESP.getFreeHeap());
    #endif
    refreshSpotify();
    parseSpotify();
  }
#endif
  
  server.begin();                              // starts webserver
  delay(100);
  
  P.displayReset();
  P.setSpriteData(pacman1, W_PMAN1, F_PMAN1, pacman2, W_PMAN2, F_PMAN2);        

  P.addChar(degreeC_ascii, degreeC);                 // adds characters for celsius and fahrenheit
  P.addChar(degreeF_ascii, degreeF);

  if(AP_established == false){
    snprintf(infowifi, sizeof(infowifi), "%s %s %s %d.%d.%d.%d",
             msgNetwork, WiFi.SSID().c_str(), msgAs, WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3]);
  } else {
    snprintf(infowifi, sizeof(infowifi), "%s %s %s %d.%d.%d.%d",
             msgAP, AP_NAME, msgWith, WiFi.softAPIP()[0], WiFi.softAPIP()[1], WiFi.softAPIP()[2], WiFi.softAPIP()[3]);
  }

  P.displayText(msgWelcome ,PA_CENTER,50,3000,PA_SCROLL_UP_LEFT,PA_SCROLL_DOWN_RIGHT);           //message shown on start up
  delay(1000);
  
  listener();                                   // server listens for request sent by html sites
  P.displayReset();  
}



void loop(void){

#ifdef ESP32                      // necessary only for ESP32 to reconnect if WiFi connection was lost
  if(AP_established == false){
      if (WiFi.status() != WL_CONNECTED) wifiReconnect();
  }
#endif
  
  server.handleClient();
  
  static uint8_t i = 0;           //counter table sCatalog
  static uint8_t si = 0;          //sprite in  -> position in table bildlein
  static uint8_t so = 0;          //sprite out -> position in table bildlein
  static uint8_t a = 0;           //counter table maintable.wetcur
  static uint8_t b = 0;           //counter table maintable.newscur

if (timeOnly==1) {                // if all messages except time are deactivated
   displayOnlyTime();
   i=1;                           // resets message loop to start position 1 (for return out of 'time only' mode into normal mode)
} else {                          // if time AND other messages are activated --> MAIN ROUTINE and STANDARD CASE
  
  displayTime();  

  if (millis() - previousweathercall > (10* 60000) && P.displayAnimate()){          // getting new weather data from server every 10 minutes
    if (enableWeath == 1){
      refreshWeather();
    }
    previousweathercall = millis();
  }


  if (millis() - previousnewscall > (60* 60000) && P.displayAnimate()){            // getting new news data from server and synchronising time every 60 minutes
   if(AP_established == false){ 
    getTimeFromServer();
   }
   if (enableNews1 == 1 || enableNews2 == 1 ){
      refreshNews();
    }
    previousnewscall = millis();
  }
    
#ifdef SPOTIFY
   if (enableSpotify == 1){
    if (millis() - previousrefresh > (1000 * expiretime ) && P.displayAnimate()){                       // calls new token from spotify server when expire time has run out
    refreshSpotify();
    previousrefresh = millis();
       }
   }
#endif

  

//// MAIN DISPLAY ROUTINE ////

if (P.displayAnimate())                                                           // animates and returns true when an animation is completed
  { 
    P.displayClear();                                                             // clears display before animation (necessary for return out of 'time only' mode)
    delay(catalog[i].delay * delayTime);                                          // delay between messages
    P.setFont(tobFont);                                                           // Parola standard font set


    if (catalog[i].effectin >= 28)                                                // picks sprites from table bildlein according to chosen effect
    si = catalog[i].effectin - 28;

    if (catalog[i].effectout >= 28)
    so = catalog[i].effectout - 28;

    P.setSpriteData(sprite[si].data, sprite[si].width, sprite[si].frames,         // entry sprite
                      sprite[so].data, sprite[so].width, sprite[so].frames);      // exit sprite

    P.displayText(catalog[i].psz, justlist[catalog[i].just], (catalog[i].speed * speedTime) , (catalog[i].pause * pauseTime), effectlist[catalog[i].effectin],
                  effectlist[catalog[i].effectout]);


   for (uint8_t m=0; m<4; m++) {                                                 // loads custom messages and configuration from array sCatalogTEMP into SHOW ARRAY sCatalog
        if (i==(location[m]-1)){ 
            if (enablemessage[m]==1){
                utf8AsciiConvert(messages[m],showmessages[m]);
                catalog[location[m]].effectin = catalogTEMP[m].effectinTEMP;
                catalog[location[m]].effectout = catalogTEMP[m].effectoutTEMP;
                catalog[location[m]].just = catalogTEMP[m].justTEMP;;
                catalog[location[m]].speed = catalogTEMP[m].speedTEMP;;
                catalog[location[m]].pause = catalogTEMP[m].pauseTEMP;;
                catalog[location[m]].delay = catalogTEMP[m].delayTEMP;
            } else {
                snprintf(showmessages[m], sizeof(showmessages[m]), "");
                catalog[location[m]].effectin = 0;
                catalog[location[m]].effectout = 26;
                catalog[location[m]].just = 1;
                catalog[location[m]].speed = 0;
                catalog[location[m]].pause = 0;
                catalog[location[m]].delay = 0;
            }
         }
       }
    
    if (i==locationTime) P.setFont(numeric7Seg);                                  // special font set only for numbers in time display
      
    if (i==(locationWeath-1)) {
        if (enableWeath == 1){
          snprintf(showweather, sizeof(showweather), "%s",  table[a].wetcur);     // loading weather info into "showweather"
          a++;
       }
    }

    if (i==(locationNews[0]-1)) {                                                 // loading news into "shownews1"
        if (enableNews1 == 1){
          utf8AsciiConvert(table[b].newscur,shownews);
          b++;
      }
    }

    if (i==(locationNews[1]-1)) {                                                 // loading news into "shownews2"
        if (enableNews2 == 1){
          utf8AsciiConvert(table[b].newscur,shownews2);
          b++;
      }
    }

    if (i==(locationOwn)){                                                        // loading and converting ownmessage into showownmessage if enabled
        if (enableOwn == 1){
          utf8AsciiConvert(ownmessage, showownmessage);
         }
    }

  #ifdef SPOTIFY
    if (i==locationSpot){                                                          // calling spotify and loading information about currently playing song if enabled
        if (enableSpotify == 1){
          parseSpotify();
         }
    }
  #endif
   
    i++;                                                                           // show next message of main array
    P.displayReset();
    
    if (i == ARRAY_SIZE(catalog)) i= 1;                                            // loop main array without first message
     
    if (a == numberofcities*2) a= 0;                                               // loop weather data
 
    if (b == ARRAY_SIZE(table)) b = 0;                                             // loop news data
  }
 }    
}
