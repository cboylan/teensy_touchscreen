
###Teensy touchpad firmware  
Fall 2010

Firmware for Teensy touchpad device.  

###Firmware

The firmware interacts with 4 teensy pins, specified in the next section, 
according to the touchscreen specs.  


![touchscreen](https://github.com/cboylan/teensy_touchscreen/raw/master/firmware/docs/bussbarssmall.jpg)
Image taken from http://www.sparkfun.com/tutorials/139


The algorithm works as follows:  

* Timer is enabled and set to spcific period  
* The timer ISR implements a state machine with four states where one state outputs
  the necessary values to read from particular axis then the next state which is 
  activated in the next ISR does the actual reading, then the next state prepares for 
  reading from the other axis, and so on.
* The firmware discards the lowest bit to reduce the noise.
* The difference between the current reading and the previous reading (relative 
  movement) is sent to the driver to move the mouse cursor.
* Scaling factor exist to control the cursor speed on Linux.


#### Source files

* firmware.c main source file that contain all of touchpad handling functionality
* print.[ch] used for dubugging
* usb_mouse.[ch] All of the usb mouse support functionality
* usb_mouse_debug.[ch] Same as the previous one with debug hooks where the firmware 
	can print debug message which is sent to the host, where hid_listen utility 
	can display them. 

Note that either usb_mouse.c or usb_mouse_debug.c should be linked to firmware.c


###Hardware 

####Components
* Nintendo DS Touch Screen from [dealexterme.com](http://www.dealextreme.com/details.dx/sku.3245)
* Nintendo DS Touch Screen Connector Breakout from [sparkfun.com](http://www.sparkfun.com/products/9170) 
* Push button
* 10k Resistor
* Breadboard and Wires for the prototype


####Connections and Pin Assignment
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


#### Testing
Firmware has been tested on Teensy 2.0 
Driver has been tested on a system running Debian GNU/Linux 5.0 Lenny with 2.6.24 kernel
