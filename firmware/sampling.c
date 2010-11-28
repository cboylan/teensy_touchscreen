
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include "sampling.h"

#define BUFSIZE 50

static volatile uint8_t head, tail;
static volatile int16_t buffer[BUFSIZE];

void adc_start(uint8_t mux, uint8_t aref)
{
	ADCSRA = (1<<ADEN) | ADC_PRESCALER;	// enable the ADC, interrupt disabled
	ADCSRB = (1<<ADHSM) | (mux & 0x20);
	ADMUX = aref | (mux & 0x1F);		// configure mux and ref
	head = 0;				// clear the buffer
	tail = 0;				// and then begin auto trigger mode
	ADCSRA = (1<<ADSC) | (1<<ADEN) | (1<<ADATE) | (1<<ADIE) | ADC_PRESCALER;
	sei();
}

uint8_t adc_available(void)
{
	uint8_t h, t;

	h = head;
	t = tail;
	if (h >= t) return h = t;
	return BUFSIZE + h - t;
}

int16_t adc_read(void)
{
	uint8_t h, t;
	int16_t val;

	do {
		h = head;
		t = tail;		// wait for data in buffer
	} while (h == t);
	if (++t >= BUFSIZE) t = 0;
	val = buffer[t];		// remove 1 sample from buffer
	tail = t;
	return val;
}

ISR(ADC_vect)
{
	uint8_t h;
	int16_t val;

	val = ADC;			// grab new reading from ADC
	h = head + 1;
	if (h >= BUFSIZE) h = 0;
	if (h != tail) {		// if the buffer isn't full
		buffer[h] = val;	// put new data into buffer
		head = h;
	}
}

