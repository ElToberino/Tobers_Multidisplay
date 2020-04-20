# Tobers Multidisplay for ESP8266 and ESP32
Tobers Multidisplay is a LED-Matrix display made of MAX7219 LED matrix modules and controlled by an ESP8266 or an ESP32. It can show current time and date, news, current weather & forecast, up to four custom messages, one guest message and - if ESP32 - information about the song currently playing on Spotify.

Tobers Multidisplay is widely and easily configurable via web interface.<br><br>
<img src="showcase/Multidisplay_final.jpg" width="500">
<br><br>

**Inspiration**<br>
Being fascinated by all kind of LED matrixes and always searching the web for inspiration, I discovered the fantastic [Parola Library](https://github.com/MajicDesigns/MD_Parola), which makes it easy to get nice text animations on a matrix.
There is a great number of projects online basing on this library - all kinds of clocks, message displays, news displays etc. A really nice one is the [Display made by ericBcreator](https://www.hackster.io/ericBcreator/1024-led-matrix-wifi-message-board-with-menu-web-interface-1b2666), which was a significant inspiration for my project. In the beginning it was my goal to get something like this running on an ESP8266. Finally it endend up with a - under the hood - completely different project. One thing I appreciated very much about the cited project and that I adopted for my own project is the idea of implementing a "local language" definition in the code, which makes it very simple to set up the display for other languages.
<br><br>

**Requirements**<br>
* *Hardware*<br>
ESP8266 with 4MB flash Memory orr ESP32 (I recommend development modules like NodeMCU e.g.)<br>
Max7219 8x8 LED matrix modules (My display is made of eight modules)<br>

* *Arduino IDE and the following Libraries:*<br>
[MAX72xx Library by majicDesigns](https://github.com/MajicDesigns/MD_MAX72XX)<br>
[Parola Library by majicDesigns](https://github.com/MajicDesigns/MD_Parola)<br>
[Arduino Json library by Benoit Blanchon](https://github.com/bblanchon/ArduinoJson)<br>
[My fork of WifiManager library (development branch) by tzapu/tablatronix](https://github.com/ElToberino/WiFiManager_for_Multidisplay)<br>

* *Installed Boards:*<br>
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
* Special thanks to Marco Colli (MajicDesigns) for his libraries, the excellent documentation and the support via arduino forum.
* Special thanks to Benoit Blanchon for his great Arduino Json Library and his friendly support.
* Local language concept and some parts of weather functions inspired by ericBcreator and his great display project
* SPIFFS administration taken and adopted from the great Arduino ESP website https://fipsok.de/ by Jens Fleischer
* HTML background pattern graphic by Henry Daubrez, taken from http://thepatternlibrary.com/<br>
Thanks to the many, many other programmers and enthusiasts in the web whose work and helpfulness enabled me to realize such a project.<br>
<br><br>
**Some impressions of the web interface:**<br>
<img src="showcase/public.jpg" width="250">&nbsp;<img src="showcase/admin.jpg" width="250">&nbsp;<img src="showcase/config.jpg" width="250">
<br>
