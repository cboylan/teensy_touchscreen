/* Mouse example with debug channel, for Teensy USB Development Board
 * http://www.pjrc.com/teensy/usb_mouse.html
 * Copyright (c) 2009 PJRC.COM, LLC
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include "sampling.h"

//#define DEBUG

#ifdef DEBUG
#include "usb_mouse_debug.h"
#include "print.h"
#else 
#include "usb_mouse.h"
#endif


#define LED_CONFIG	(DDRD |= (1<<6))
#define LED_ON		(PORTD &= ~(1<<6))
#define LED_OFF		(PORTD |= (1<<6))
#define CPU_PRESCALE(n)	(CLKPR = 0x80, CLKPR = (n))

uint8_t read_x();
uint8_t read_y();

int main(void)
{
	uint8_t x=0, y =0;
	uint8_t px, py;
	uint8_t initalized = 0;
        int8_t deltax, deltay;

	// set for 16 MHz clock
	CPU_PRESCALE(0);
	LED_CONFIG;
	LED_OFF;

	// Initialize the USB, and then wait for the host to set configuration.
	// If the Teensy is powered without a PC connected to the USB port,
	// this will wait forever.
	usb_init();
	while (!usb_configured()) /* wait */ ;

	// Wait an extra second for the PC's operating system to load drivers
	// and do whatever it does to actually be ready for input
	_delay_ms(1000);

	//adc_start(ADC_MUX_PIN_D7, ADC_REF_POWER);
	while (1) {

		x = read_x();
		y = read_y();

		if (!initalized) {
		  px = x;
		  py = y;
		  initalized = 1;
		} else {
		  deltax = px - x;
		  deltay = py - y;
		  if (((deltax < 18) &&  (deltax > -18))
			&& (deltay < 18 && deltay > -18)) {

#ifdef DEBUG
		    if (deltax != 0 || deltay != 0) {
		      print("x="); phex(deltax); print("\t");
		      print("y="); phex(py - y); print("\n");
		    }
#endif
		    usb_mouse_move(deltax, deltay, 0);
		  }
		  px = x;
		  py = y;
		}
	}
}

uint8_t read_x(void)
{
    uint8_t l,h;

    DDRF = 0b00100010; // Output on F1(5V) and F5(GND), Input on F4(ADC)
    PORTF |= _BV(1);
    PORTF &= ~(_BV(5));
    
    PORTF |= _BV(6); // pullup resistor

   
    _delay_ms(50); //wait for screen to initialize
   
    ADMUX = (1 << REFS0) | (1 << MUX2); //ADC4
    ADCSRA = (1 << ADEN)|(1 << ADSC)|(1<< ADC_PRESCALER);

    while(ADCSRA & (1 << ADSC));
    l = ADCL;
    h = ADCH & 0x03;
    h = h << 6;
    //h = h + l;
    h = h | (l >> 2); 
    return h;
}

uint8_t read_y(void)
{
    uint8_t l,h;

    DDRF = 0b01010000; // Output on F4(5V) and F6(GND), Input on F1(ADC)
    PORTF |= _BV(4);
    PORTF &= ~(_BV(6));

    PORTF |= _BV(5); // pullup resistor
   
    _delay_ms(50); //wait for screen to initialize
   
    ADMUX = (1<< REFS0) | (1 << MUX0); //ADC1
    ADCSRA = (1 << ADEN)|(1 << ADSC)|(1<< ADC_PRESCALER);

    while(ADCSRA & (1 << ADSC));
    l = ADCL;
    h = ADCH & 0x03;
    h = h << 6;
    h = h | (l >> 2);

    return h;
}
