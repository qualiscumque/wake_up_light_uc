/*
 * WakeUpLight.c
 *
 * Created: 13.10.2016 15:39:11
 * Author : Ralf
 */ 

#ifndef F_CPU
#define F_CPU 8000000UL
#endif

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "I2C_slave.h"

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
	DDRD = 0xFF; //Mark PORTC as output
	DDRB |= 1 << PB1;
	DDRB |= 1 << PB2;	
}

void my_delay (uint16_t milliseconds)
{
	for (; milliseconds > 0; milliseconds--)
	_delay_ms (1);
}

void pwm_up_down (const uint16_t pwm_table[], int16_t size, uint16_t delay)
{
	int16_t tmp;
	
	for (tmp = 0; tmp < size; tmp++)
	{
		OCR1A = pgm_read_word (& pwm_table[tmp]);
		my_delay (delay);
	}
	
	for (tmp = size-1; tmp >= 0; tmp--)
	{
		OCR1A = pgm_read_word (& pwm_table[tmp]);
		my_delay (delay);
	}
}

void pwm_16_256 (uint16_t delay)
{
	// 16 Bit Fast PWM
	TCCR1A = (1 << WGM11) | (1 << COM1A1);
	// stop timer
	TCCR1B = 0;
	// TOP for PWM, full 16 Bit
	ICR1 = 0xFFFF;
	// prescaler 1 -> ~122 Hz PWM frequency
	TCCR1B = (1 << WGM12) | (1 << WGM13) | 1;
	
	pwm_up_down(pwmtable_16, 256, delay);
}

int main(void)
{
    int16_t step_time = 400;
		
    ioinit();
	
	I2C_init(0x32); // initialize as slave with address 0x32
	
	// allow interrupts
	sei();
		
    while (1) 
    {
	
		if (rxbuffer[0] != 0)
		{
			
			if (rxbuffer[0] == 1)
			{
				//pwm_16_256(step_time/16, 1);
				pwm_16_256(step_time/16);
			}
			rxbuffer[0] = 0;			
		}

		//PORTD = 0x00;
		//_delay_ms(500);
		//PORTD = 0xFF;
		//_delay_ms(500);
    }
}

