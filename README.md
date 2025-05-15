# FeuCamp
Wifi-controlled Fire camp simulator.


A Pico-based Fraise board drives a string of WS2811 RGB lights, and generates a fire-like sound (random-VCA pink noise + cracks).

Another Fraise board, equiped with a PicoW, opens a web server (and a web socket server), either connecting to a local Wifi access point or providing its own one, and allows to control the app board.


