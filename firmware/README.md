<p>Teensy teouchpad firmware  
Fall 2010</p>

<p>Firmware for the Teensy touchpad device.  
Tested on Debian GNU/Linux 5.0 Lenny with 2.6.24 kernel</p>

<h1>Firmware</h1>

<p>The firmware interacts with 4 teensy pins, specified in the next section, 
according to the touchscreen specs.</p>
![touchscreen](https://github.com/cboylan/teensy_touchscreen/raw/master/firmware/docs/bussbarslarge.jpg)
Image taken from
 http://www.sparkfun.com/tutorials/139</p>

<h1>Hardware connections</h1>

<p>Pin Assignment:
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

<p>A push button is connected to the teensy D0 pin, when pressed the firmware
send a left click to the driver.</p>
