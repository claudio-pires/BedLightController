# BedLightController
A simple Homespan based light controller for my bedroom


I recently did some renovations in my bed room and decided to implement smart home light control.
My hobby is to work with Arduino and ESP32. And both my family and me use iphone.

I was played in the past with the Homespan library https://github.com/HomeSpan/HomeSpan just running the examples. So Ive decided to develop  
my own light controller using ESP32S and Homespan.

Previous to the renovation my bedroom had a ceiling lamp, controled by a wall switch, and two reading lamps on top on the side night tables, each one controlled by their own switch.

Ive added a new floor stand lamp and a bed headboard that includes the possibility to put in back a new lamp. 

So the idea was:
Control each lamp from the phone and homepod.
Keep the possibility to turn on/off the ceiling lampo using the wall switch. So for this we changed the swith from two position one to a like pushbutton type. Same for reading lamps.
From this wall switch we must have the possibility to turn off all lights, and the possibility to turn on also the stand lamps.
Finally, the headboard light is a dimmeable RGB led. 


