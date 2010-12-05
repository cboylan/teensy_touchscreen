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
#include <avr/interrupt.h>
#include <util/delay.h>

#ifdef DEBUG
#include "usb_mouse_debug.h"
#include "print.h"
#else 
#include "usb_mouse.h"
#endif

#define LED_CONFIG  (DDRD |= (1<<6))
#define LED_ON      (PORTD &= ~(1<<6))
#define LED_OFF     (PORTD |= (1<<6))
#define CPU_PRESCALE(n) (CLKPR = 0x80, CLKPR = (n))

#define ADC_PRESCALER_64 ((1<<ADPS2) | (1<<ADPS1))


enum
{
    STATE_X_READ,
    STATE_Y_START,
    STATE_Y_READ,
    STATE_X_START
} Touchscreen_States;

volatile uint16_t currentX, previousX;
volatile uint16_t currentY, previousY;
uint8_t currentState = STATE_X_READ;
uint8_t initalized = 0;

int8_t deltaX, deltaY;

/* previous button state, and current button state 
   used to detect click
*/
char currButtonState = 0x1; // initial state not pressed

/* HACK, current coordinats */
//#define COORD 'X'


uint8_t read_x(void);
uint8_t read_y(void);
void move_mouse(void);
void SetupTouchscreen(void);

int main(void)
{
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

    // Setup the timer interrupt that handles the touch screen,
    // and setup the ADC
    SetupTouchscreen();

    // initialize PIN D0 as digital input, others are set to 
    // pullup resistor
    DDRD = 0b00000000; 
    DDRD = 0b11111110; 

    // Enable interrupts
    sei();

    while (1) {}
}


/* Setup code for the touchscreen ADC and Timer */
void SetupTouchscreen(void)
{
    /* ADC setup for single conversions */
    //ADC_Init(ADC_RIGHT_ADJUSTED | ADC_PRESCALE_64 | ADC_REFERENCE_AREF | ADC_SINGLE_CONVERSION); 
    ADCSRA = (( 1 << ADEN) | ADC_PRESCALER_64);
    

    /* Timer setup for 100Hz interrupts */
    TCCR1B |= (1 << WGM12); // Configure timer 1 for CTC mode
    //TCCR1A |= (1 << COM1A0); // Enable timer 1 Compare Output channel A in toggle mode
    TIMSK1 |= (1 << OCIE1A); 
    OCR1A   = 0x09C4; // Set CTC compare value to 100Hz at 16MHz AVR clock, with a prescaler of 64
    TCCR1B |= ((1 << CS10) | (1 << CS11)); // Start timer at Fcpu/64
}

/* Setup Y coordinates
   Pin F4  -> 5V
   Pin F6  -> GND
   Pin F1  -> ADC
   Pin F5  -> floating
*/

void setupY(void) 
{
    // SetupY 
    DDRF = 0b01010000; // Output on F4(5V) and F6(GND), Input on F1(ADC)
    PORTF = _BV(4);
    PORTF &= ~(_BV(6));
    PORTF |= _BV(5); // pullup resistor
}

/* Setup X Coordinates 
    Pin F1 -> 5V
    Pin F5 -> GND
    Pin F4 -> ADC
    Pin F6 -> floating
*/
void setupX()
{
    // SetupX
    DDRF = 0b00100010; // Output on F1(5V) and F5(GND), Input on F4(ADC)
    PORTF = _BV(1);
    PORTF &= ~(_BV(5));
    PORTF |= _BV(6); // pullup resistor
}

/* return the result of the ADC */

uint16_t getResult()
{
    uint16_t result;

    //while(ADCSRA & (1 << ADSC));
    ADCSRA |= (1 << ADIF); // Clear the flag

    result = ADC;

    return result;
}

/* Timer compare ISR, for ADC reads */
ISR(TIMER1_COMPA_vect, ISR_BLOCK)
{
    switch (currentState)
    {
        case STATE_X_READ:
            currentX = getResult();
#ifdef DEBUG
            phex16(currentX);
#endif           
#if defined COORD && COORD == 'X'
            setupX();
            currentState = STATE_X_START;
            move_mouse();
#else
            setupY();
            currentState = STATE_Y_START;
#endif
            break;
        case STATE_Y_START:
            ADMUX = (1<< REFS0) | (1 << MUX0); //ADC1
            ADCSRA = (1 << ADEN)|(1 << ADSC)|(1<< ADC_PRESCALER_64);
            
            currentState = STATE_Y_READ;
            break;
        case STATE_Y_READ:
            currentY = getResult();
#ifdef DEBUG
            phex16(currentY); print("\t");
#endif
            
            move_mouse();
           
#if defined COORD && COORD == 'Y'
            setupY();
            currentState = STATE_Y_START;
#else
            setupX();
            currentState = STATE_X_START;
#endif
            break;
        case STATE_X_START:
            ADMUX = (1 << REFS0) | (1 << MUX2); //ADC4
            ADCSRA = (1 << ADEN)|(1 << ADSC)|(1<< ADC_PRESCALER_64);
           
            currentState = STATE_X_READ;
            break;
    }
} 


void move_mouse(void) {

    /* don't move the mouse unless we already took 2 readings */
    if (!initalized) {
        initalized = 1;
    } else {
        // ignore least significant bit, as it's probably noise
        currentX = currentX >> 1; 
        currentY = currentY >> 1;

        deltaX = previousX - currentX;
        deltaY = previousY - currentY;
    
        if (((deltaX < 18) &&  (deltaX > -18))
            && (deltaY < 18 && (deltaY > -18))) {
        
#ifdef DEBUG
            if (deltaX != 0 || deltaY != 0) {
              print("x="); phex(deltaX); print("\t");
              print("y="); phex(deltaY); print("\n");
            }
#endif
            usb_mouse_move(deltaX, deltaY, 0);
            currButtonState = PIND & 0x01;  // Care about D0 only
            // button pressed
            if (!currButtonState) {
                usb_mouse_buttons(1, 0, 0);
            } else { // released
                usb_mouse_buttons(0, 0, 0);
            }
        }
    }
            
    previousX = currentX;
    previousY = currentY;
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
    ADCSRA = (1 << ADEN)|(1 << ADSC)|(1<< ADC_PRESCALER_64);

    while(ADCSRA & (1 << ADSC));
    l = ADCL;
    h = ADCH & 0x03;
    h = h << 6;
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
    ADCSRA = (1 << ADEN)|(1 << ADSC)|(1<< ADC_PRESCALER_64);

    while(ADCSRA & (1 << ADSC));
    l = ADCL;
    h = ADCH & 0x03;
    h = h << 6;
    h = h | (l >> 2);

    return h;
}
