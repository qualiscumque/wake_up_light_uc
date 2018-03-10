/*
 * WakeUpLight.c
 *
 * Created: 13.10.2016 15:39:11
 * Author : Ralf
 */ 

#ifndef F_CPU
#define F_CPU 8000000UL
#endif

#define F_PWM 100                       // PWM-Frequenz in Hz
#define PWM_STEPS 256                   // PWM-Schritte pro Zyklus(1..256)
#define PWM_PORT PORTD                  // Port für PWM
#define PWM_DDR DDRD                    // Datenrichtungsregister für PWM

// ab hier nichts ändern, wird alles berechnet

#define T_PWM (F_CPU/(F_PWM*PWM_STEPS)) // Systemtakte pro PWM-Takt

#if (T_PWM<(93+5))
#error T_PWM zu klein, F_CPU muss vergroessert werden oder F_PWM oder PWM_STEPS verkleinert werden
#endif

// includes

#include <stdint.h>
#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>

// globale Variablen

volatile uint8_t pwm_setting[4];                    // Einstellungen für die einzelnen PWM-Kanäle

// Timer 1 Output COMPARE A Interrupt

ISR(TIMER1_COMPA_vect) {
	static uint8_t pwm_cnt=0;
	uint8_t tmp=0;

	OCR1A += (uint16_t)T_PWM;
	
	if (pwm_setting[0] > pwm_cnt) tmp |= (1<<0);
	if (pwm_setting[1] > pwm_cnt) tmp |= (1<<1);
	if (pwm_setting[2] > pwm_cnt) tmp |= (1<<2);
	if (pwm_setting[3] > pwm_cnt) tmp |= (1<<3);
	//if (pwm_setting[4] > pwm_cnt) tmp |= (1<<4);
	//if (pwm_setting[5] > pwm_cnt) tmp |= (1<<5);
	//if (pwm_setting[6] > pwm_cnt) tmp |= (1<<6);
	//if (pwm_setting[7] > pwm_cnt) tmp |= (1<<7);
	PWM_PORT = tmp;                         // PWMs aktualisieren
	if (pwm_cnt==(uint8_t)(PWM_STEPS-1))
	pwm_cnt=0;
	else
	pwm_cnt++;
}

int main(void) {

	// PWM einstellen
	
	PWM_DDR = 0xFF;         // Port als Ausgang
	
	// Timer 1 OCRA1, als variablem Timer nutzen

	TCCR1B = 1;             // Timer läuft mit vollem Systemtakt
	TIMSK1 |= (1<<OCIE1A);   // Interrupt freischalten

	sei();                  // Interrupts global einschalten

	///*********************************************************************/
	//// nur zum Testen, im Anwendungsfall löschen
//
	//volatile uint8_t tmp;
	//const uint8_t t1[8]={27, 40, 3, 17, 150, 99, 5, 9};
	//const uint8_t t2[8]={27, 40, 3, 0, 150, 99, 5, 9};
	//const uint8_t t3[8]={27, 40, 3, 17, 3, 99, 3, 0};
	const uint8_t t4[8]={10, 10, 10, 10, 10, 10, 10, 10};
	//const uint8_t t5[8]={0, 0, 0, 0, 0, 0, 0, 9};
	//const uint8_t t6[8]={33, 33, 33, 33, 33, 33, 33, 33};
//
	//// Messung der Interruptdauer
	//tmp =0;
	//tmp =0;
	//tmp =0;
//
	//// Debug
//
	//memcpy(pwm_setting, t1, 8);
	//
	//memcpy(pwm_setting, t2, 8);
//
	//memcpy(pwm_setting, t3, 8);
//
	memcpy(pwm_setting, t4, 8);
//
	//memcpy(pwm_setting, t5, 8);
	//
	//memcpy(pwm_setting, t6, 8);
//
	///*********************************************************************/

	while(1)
	{
	};

	return 0;
}

