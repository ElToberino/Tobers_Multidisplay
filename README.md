# Tobers Multidisplay for ESP8266 and ESP32
Tobers Multidisplay is a LED matrix display made of MAX7219 LED matrix modules and controlled by an ESP8266 or an ESP32. It is based on the fantastic [Parola library](https://github.com/MajicDesigns/MD_MAX72XX) by majicDesigns and can show current time and date, news, current weather & forecast, up to four custom messages, one guest message and - if ESP32 - information about the song currently playing on Spotify.

Tobers Multidisplay is widely and easily configurable via web interface.<br><br>
<img src="showcase/Multidisplay_final.jpg" width="500">
<br><br>


**Requirements**<br>
* *Hardware*<br>
ESP8266 with 4MB flash memory or ESP32 (I recommend development modules like NodeMCU e.g.)<br>
Max7219 8x8 LED matrix modules (My display is made of eight modules)<br>

* *Arduino IDE and the following libraries:*<br>
[MAX72xx Library by majicDesigns](https://github.com/MajicDesigns/MD_MAX72XX)<br>
[Parola Library by majicDesigns](https://github.com/MajicDesigns/MD_Parola)<br>
[Arduino Json library by Benoit Blanchon](https://github.com/bblanchon/ArduinoJson)<br>
[My fork of WifiManager library (development branch) by tzapu/tablatronix](https://github.com/ElToberino/WiFiManager_for_Multidisplay)<br>

* *Installed boards:*<br>
[ESP8266 core for Arduino](https://github.com/esp8266/Arduino)<br>
[ESP32 core for Arduino](https://github.com/espressif/arduino-esp32)<br>

* *Recommended tools:*<br>
[ESP8266 Sketch Data Upload](https://github.com/esp8266/arduino-esp8266fs-plugin)<br>
[ESP8266 Sketch Data Upload](https://github.com/me-no-dev/arduino-esp32fs-plugin)<br>

* *Required accounts:*<br>
WEATHER: personal api key from [openweathermap.org](https://openweathermap.org/)<br>
NEWS: personal api key from [newsapi.org/](https://newsapi.org/)<br>
SPOTIFY: (free or premium) account AND [developer registration of your device](https://developer.spotify.com/dashboard/)<br>
<br>

**For complete an detailed information about the setup of the device and the steps to be taken before compiling please visit the [article I wrote at hacktser.io](https://www.hackster.io/eltoberino/tobers-multidisplay-for-esp8266-and-esp32-17cac9). You will also find some more pictures and a video there.**<br><br>

**Credits**<br>
This project wouldn't have been possible without the work of many others:
* Special thanks to Marco Colli (MajicDesigns) for his libraries, the excellent documentation and the support via arduino forum
* Special thanks to Benoit Blanchon for his great Arduino Json Library and his friendly support
* Local language concept and some parts of weather functions inspired by ericBcreator and his [really nice display project](https://www.hackster.io/ericBcreator/1024-led-matrix-wifi-message-board-with-menu-web-interface-1b2666)
* SPIFFS administration taken and adopted from the great Arduino ESP website https://fipsok.de/ by Jens Fleischer
* HTML background pattern graphic by Henry Daubrez, taken from http://thepatternlibrary.com/ <br>

Thanks to the many, many other programmers and enthusiasts in the web whose work and helpfulness enabled me to realize such a project.<br>
<br><br>
**Some impressions of the web interface:**<br>
<img src="showcase/public.jpg" height="250">&nbsp;&nbsp;<img src="showcase/admin.jpg" height="250">&nbsp;&nbsp;<img src="showcase/config.jpg" height="250">
<br>
<br><br>
**Known Issues**<br>
<br>
*ESP32*<br>
Unfortunately I faced some issues running the code on an ESP32. Searching the web I found out I'm not the only one having these problems...<br>
The most annoying issue is [related to the webserver](https://github.com/espressif/arduino-esp32/issues/3890): sending the guest message via index.html or updating the messages via admin.html can result in a failure indication on the website, though the messages have been sent successfully - a page reload shows that. Beside that, the call of the html sites "news.html" and "spiffs.html" can partially fail - **but only with Chromium based browsers. With Firefox everything works fine** - I can confirm this for Windows and Android. Note: This doesn't affect the messages shown on display - they are running without any problems!<br>
(It would be nice if someone had an idea how to get this running or how to set up a workaround for this issue.)<br>
<br>
Sometimes after skipping the config portal ( - regular setup with Wifi credentials works fine - ) the webserver can't be reached, though the AP has been established. Power cycling the ESP32 can help. <br>
*I guess this could be related to another known issue [connection only every second time](https://github.com/espressif/arduino-esp32/issues/2501#). You won't notice that issue, because I made a workaround for this in the code.*<br>
<br>
*ESP8266*<br>
In some rare cases, there can occur some heap problems calling weather Data - but only if debug mode is active. So if you don't need it, disable debugging and everything works fine. (I was able to confirm this, writing the successful calls of weather data into a log file for several days.)
