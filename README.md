# Boostuino
A software written in cpp to use an arduino nano as a boost controller

This is currently untested and unfinished

This code will turn your Arduino nano into a boost controller (with the addition of some hardware)  
Hardware needed:  
128x64 SSD1306 I2C OLED LCD (https://www.amazon.com/HiLetgo-Serial-SSH1106-Display-Arduino/dp/B01MRR4LVE)  
Bosche 3BAR map sensor (barbed) (https://www.reveltronics.com/en/shop/70/6/onboard-computers/boost-pressure-sensor-map-3bar-5v-detail)  
3 port electronic mac valve (https://www.amazon.com/Solenoid-Electronic-Control-Fittings-35A-AAA-DDBA-1BA/dp/B088FXFG1G)  
MOSFET trigger board (https://www.amazon.com/DEVMO-High-Power-Adjustment-Electronic-Brightness/dp/B0899XYBYB)  
momentary switches/buttons (4) (https://www.amazon.com/Twidec-Waterproof-Momentary-Pre-soldered-PBS-33B-BK-X/dp/B08JHW8BPV)  
you will also need resistors, I have not yet determined the resistance  

this boost controller works differently than most electonic controllers on the market.

The configuration settings for this are below:
Preferred Unit
Spring Pressure
Crack Pressure
Max Boost
Scale

Preferred unit lets you choose which unit you would prefer for it to show you on the boost gauge, this can be BAR, PSI, or kPa

Spring pressure is where you put in what the spring currently in the wastegate is rated for (used for calculating wastegate "openness" for smoother operation)

Crack Pressure is the pressure you want the wastegate to begin opening, the closer this number to your max boost, the faster you build boost

Max Boost is your max boost, the wastegate will be fully opened past this point

Scale lets you choose the rate the wastegate is opened between Crack Pressure and Max Boost, this can be linear, x^2, x^3, or x^4

There are 4 buttons on the controller, they are "cycle", "next", "up", and "down"

"cycle" cycles through the different option screens

"next" moves the "cursor" to the right as you're inputting the different values for the boost levels, the cursor over the unit lets you change the unit used for the boost setting options outside of the main display unit

"up" and "down" cycle through the options on the current screen or cursor location

Any values that are "impossible" (max boost is lower than crack pressure or spring pressure), will be snapped to an appropriate value on setting changed (equal to the highest of the 2)
