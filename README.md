# Tobers Multidisplay for ESP8266 and ESP32
Tobers Multidisplay is a LED-Matrix display controlled by an ESP8266 or an ESP32. It can show current time and date, news, current weather & forecast, up to four custom messages, one guest message and - if ESP32 - information about the song currently playing on Spotify.

Tobers Multidisplay is widely and easily configurable via web interface.
<img src="showcase/Multidisplay_final.jpg" width="500">
<br><br>

**Inspiration**<br>
Being fascinated by all kind of LED matrixes and always searching the web for inspiration, I discovered the fantastic [Parola Library](https://github.com/MajicDesigns/MD_Parola), which makes it easy to get nice text animations on a matrix.
There is a great number of projects online basing on this library - all kinds of clocks, message displays, news displays etc. A really nice one is the [Display made by ericBcreator](https://www.hackster.io/ericBcreator/1024-led-matrix-wifi-message-board-with-menu-web-interface-1b2666), which was a significant inspiration for my project. In the beginning it was my goal to get something like this running on an ESP8266. Finally it endend up with a - under the hood - completely different project. One thing I appreciated very much about the cited project and that I adopted for my own project is the idea of implementing a "local language" definition in the code, which makes it very simple to set up the display for other languages.
