# HomePool

Project to control a bunch of devices of my home pool through a web app. It's based on a 8266 wifi mcu module 
but also uses a P9813 controller for controlling the RGB lights.

Controlled devices:

* LCD for displaying pumps status and temperatures
* Pool filtering pump
* Pool heat pump
* Infinity pool edge pump
* RGB leds for night lightening
* Pool termometer
* Roof termometer
* A simple button for turning the filtering pump on

Pool heat pump is enabled whenever we find that Roof temperature > Pool Temperature + XoC, where X is 
a configurable temperature difference.

Both the filtering and infinity edge pumps works on a scheduled basis, with the times being configured 
through a web app.
