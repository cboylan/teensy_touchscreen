
Teensy teouchpad firmware  
Fall 2010

Firmware for the Teensy touchpad device.  
Tested on Debian GNU/Linux 5.0 Lenny with 2.6.24 kernel

###Firmware

The firmware interacts with 4 teensy pins, specified in the next section, 
according to the touchscreen specs.  


![touchscreen](https://github.com/cboylan/teensy_touchscreen/raw/master/firmware/docs/bussbarssmall.jpg)
Image taken from http://www.sparkfun.com/tutorials/139


The algorithm works as follows:
- Timer is enabled and set to spcific period
- The timer ISR implements a state machine that is responsible for 
  preparing the touchpad for the next read, then on the next interrupt do 
  the actual read


###Hardware connections


####Pin Assignment
<table>
  <tr>
  	<td>Teensy</td>
  	<td>Touchscreen</td>
  	<td>Values to get X Pos.</td>
  	<td>Values to get Y Pos.</td>
  </tr>
  <tr>
  	<td>F1</td>
  	<td>X1</td>
  	<td>5V</td>
  	<td>ADC</td>
  </tr>
  <tr>
  	<td>F4</td>
  	<td>Y2</td>
  	<td>ADC</td>
  	<td>5V</td>
  </tr>
  <tr>
  	<td>F5</td>
  	<td>X2</td>
  	<td>GND</td>  	
  	<td>floating</td>
  </tr>
  <tr>
  	<td>F6</td>
  	<td>Y1</td>
  	<td>floating</td>
  	<td>GND</td>
  </tr>

</table>

A push button is connected to the teensy D0 pin, when pressed the firmware
send a left click to the driver.

####Schematic diagram:
![teensy_touchscreen](https://github.com/cboylan/teensy_touchscreen/raw/master/firmware/docs/Teensy_Touchscreen.png)

Schematic for the teensy connections with the touchscreen