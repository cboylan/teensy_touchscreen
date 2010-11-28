#ifndef _analog_h_included__
#define _analog_h_included__

#include <stdint.h>

void adc_start(uint8_t mux, uint8_t aref);
uint8_t adc_available(void);
int16_t adc_read(void);

#define ADC_MUX_PIN_F0    0x00
#define ADC_MUX_PIN_F1    0x01
#define ADC_MUX_PIN_F2    0x02
#define ADC_MUX_PIN_F3    0x03
#define ADC_MUX_PIN_F4    0x04
#define ADC_MUX_PIN_F5    0x05
#define ADC_MUX_PIN_F6    0x06
#define ADC_MUX_PIN_F7    0x07
#define ADC_MUX_PIN_D4    0x20
#define ADC_MUX_PIN_D6    0x21
#define ADC_MUX_PIN_D7    0x22
#define ADC_MUX_PIN_B4    0x23
#define ADC_MUX_PIN_B5    0x24
#define ADC_MUX_PIN_B6    0x25

#define ADC_REF_POWER     (1<<REFS0)
#define ADC_REF_INTERNAL  ((1<<REFS1) | (1<<REFS0))
#define ADC_REF_EXTERNAL  (0)

// These prescaler values are for high speed mode, ADHSM = 1
#if F_CPU == 16000000L
#define ADC_PRESCALER ((1<<ADPS2) | (1<<ADPS1))
#elif F_CPU == 8000000L
#define ADC_PRESCALER ((1<<ADPS2) | (1<<ADPS0))
#elif F_CPU == 4000000L
#define ADC_PRESCALER ((1<<ADPS2))
#elif F_CPU == 2000000L
#define ADC_PRESCALER ((1<<ADPS1) | (1<<ADPS0))
#elif F_CPU == 1000000L
#define ADC_PRESCALER ((1<<ADPS1))
#else
#define ADC_PRESCALER ((1<<ADPS0))
#endif

// some avr-libc versions do not properly define ADHSM
#if defined(__AVR_AT90USB646__) || defined(__AVR_AT90USB1286__)
#if !defined(ADHSM)
#define ADHSM (7)
#endif
#endif

#endif
