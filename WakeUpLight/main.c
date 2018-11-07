/*
 * WakeUpLight.c
 *
 * Created: 13.10.2016 15:39:11
 * Author : Ralf/Stone
 */ 

#ifndef F_CPU
#define F_CPU 8000000UL
#endif

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include "I2C_slave.h"


// Connected PINS:
// Pin09: (PCINT21/OC0B/T1) PD5
// Pin10: (PCINT22/OC0A/AIN0) PD6
// Pin13: (PCINT1/OC1A) PB1
// Pin14: (PCINT2/SS/OC1B) PB2


// Protocol Description:
// |---------+-------------------+-------------------------------------------------------|
// | Address | Name              | Description                                           |
// |---------+-------------------+-------------------------------------------------------|
// | 0x00    | Command Field     | Command field: 0xFF = Invalidate                      |
// |         |                   | 0xCC = Clear input buffer, other values are ignored   |
// | 0x01    | Led Select        | LED number to address                                 |
// | 0x02    | Led value high    | Target brightness value for LED, 16 Bit High          |
// | 0x03    | Led value low     | Target brightness value for LED, 16 Bit High          |
// | 0x04    | Fading delay high | Fading delay value in µs, 16 Bit High                 |
// | 0x05    | Fading delay low  | Fading delay value in µs, 16 Bit High                 |
// |---------+-------------------+-------------------------------------------------------|

const uint16_t pwmtable_16[256] PROGMEM =
{
	0, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3,
	3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 6, 6, 6, 6, 7,
	7, 7, 8, 8, 8, 9, 9, 10, 10, 10, 11, 11, 12, 12, 13, 13, 14, 15,
	15, 16, 17, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
	31, 32, 33, 35, 36, 38, 40, 41, 43, 45, 47, 49, 52, 54, 56, 59,
	61, 64, 67, 70, 73, 76, 79, 83, 87, 91, 95, 99, 103, 108, 112,
	117, 123, 128, 134, 140, 146, 152, 159, 166, 173, 181, 189, 197,
	206, 215, 225, 235, 245, 256, 267, 279, 292, 304, 318, 332, 347,
	362, 378, 395, 412, 431, 450, 470, 490, 512, 535, 558, 583, 609,
	636, 664, 693, 724, 756, 790, 825, 861, 899, 939, 981, 1024, 1069,
	1117, 1166, 1218, 1272, 1328, 1387, 1448, 1512, 1579, 1649, 1722,
	1798, 1878, 1961, 2048, 2139, 2233, 2332, 2435, 2543, 2656, 2773,
	2896, 3025, 3158, 3298, 3444, 3597, 3756, 3922, 4096, 4277, 4467,
	4664, 4871, 5087, 5312, 5547, 5793, 6049, 6317, 6596, 6889, 7194,
	7512, 7845, 8192, 8555, 8933, 9329, 9742, 10173, 10624, 11094,
	11585, 12098, 12634, 13193, 13777, 14387, 15024, 15689, 16384,
	17109, 17867, 18658, 19484, 20346, 21247, 22188, 23170, 24196,
	25267, 26386, 27554, 28774, 30048, 31378, 32768, 34218, 35733,
	37315, 38967, 40693, 42494, 44376, 46340, 48392, 50534, 52772,
	55108, 57548, 60096, 62757, 65535
};

void ioinit (void)
{
	DDRD = 0xFF; //Mark PORTD as output
	DDRB = 0xFF; //Mark PORTB as output	
}

void set_pwm_value(const uint16_t pwm_table[], int16_t val)
{	
	OCR0A = pgm_read_word(&pwm_table[val >> 1]);
	OCR0B = pgm_read_word(&pwm_table[val >> 1]);
	OCR1A = pgm_read_word(&pwm_table[val]);
	OCR1B = pgm_read_word(&pwm_table[val]);
}

void setup_pwm(const uint16_t val, const uint8_t select_mask)
{
	TCCR0A = (1<<WGM02) | (1<<WGM01) | (1<<WGM00);
	if((select_mask & 0b0001) != 0) {
		TCCR0A |= (1<<COM0B1);
	}
	if((select_mask & 0b0010) != 0) {
		TCCR0A |= (1<<COM0A1);
	}
	
	TCCR0B = (0<<WGM02) | (1<<CS02) | (0<<CS01);
	
		
	// 16 Bit Fast PWM
	TCCR1A = (1 << WGM11);
	if((select_mask & 0b0100) != 0) {
		TCCR1A |= (1<<COM1A1);
	}
	if((select_mask & 0b1000) != 0) {
		TCCR1A |= (1<<COM1B1);
	}
	
	// stop timer
	TCCR1B = 0;
	// TOP for PWM, full 16 Bit
	ICR1 = 0xFFFF;
	// prescaler 1 -> ~122 Hz PWM frequency
	TCCR1B = (1 << WGM12) | (1 << WGM13) | 1;
	
	set_pwm_value(pwmtable_16, val);
}


void set_pin(volatile uint8_t* _port, uint8_t _nr)
{
	*_port |= (1 << _nr);
}

void reset_pin(volatile uint8_t* _port, uint8_t _nr)
{
	*_port &= ~(1 << _nr);
}

void switch_pin(uint8_t led_nr, uint8_t power)
{
    void (*switch_fct)(volatile uint8_t*, uint8_t) = NULL;
	switch_fct = (bool)power ? &set_pin : &reset_pin;

	switch(led_nr)
	{
		case 1:
		switch_fct(&PORTD, PD5);
		break;
		
		case 2:
		switch_fct(&PORTD, PD6);
		break;
		
		case 3:
		switch_fct(&PORTB, PB1);	
		break;
		
		case 4:
		switch_fct(&PORTB, PB2);	
		break;
	}
}


int main(void)
{
	int8_t led_nr = 0;
    uint16_t led_brightness = 0;
    uint16_t led_delay = 0;
       
	ioinit();
	I2C_init(0x32); // initialize as slave with address 0x32
	
	// allow interrupts
	sei();
		
    while (1) 
    {
		if (rxbuffer[0] == 0xFF) // Flush
		{
			led_nr = rxbuffer[1];
			led_brightness = (rxbuffer[2] << 8) | rxbuffer[3];
			led_delay = (rxbuffer[4] << 8) | rxbuffer[5];
			
			// invoke
			setup_pwm(rxbuffer[3], 1 << (led_nr -1));
				
			// clear input buffer
			memset(rxbuffer, 0, sizeof(rxbuffer));
		}
		
		if (rxbuffer[0] == 0xCC) // Clear input buffer
		{
			memset(rxbuffer, 0, sizeof(rxbuffer));
		}
    }
}

