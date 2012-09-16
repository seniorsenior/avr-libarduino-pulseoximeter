/********************************************************************
* pulse oximeter based on libarduino 
* copyright (c) 2009 Michael Spiceland
********************************************************************
**  This program is free software; you can redistribute it and/or
**  modify it under the terms of the GNU General Public License
**  as published by the Free Software Foundation; either version 2
**  of the License, or (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program; if not, write to the Free Software Foundation, 
**  Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
********************************************************************/
#include <avr/interrupt.h>
#include "tsl230.h"
#include "timercountercalcs.h"

volatile uint16_t tsl230_sample;
volatile uint16_t tsl230_sample_ready = 0;
volatile uint32_t tsl230_overflow_count = 0;

#define TSL230_BUFFERSIZE	32
#define DEBUG_PORT	PORTB
#define DEBUG_DDR	DDRB
#define DEBUG_PIN	2

ISR(INT0_vect)
{
	if (PIND & _BV(2)) // rising edge
	{
		//setpin(DEBUG_PORT, DEBUG_PIN);
		TCNT1 = 0; // reset counter
		onboard_led_off();
	}
	else // falling edge
	{
		//clearpin(DEBUG_PORT, DEBUG_PIN);
		tsl230_sample = TCNT1;
		tsl230_sample_ready = 1;
	}
}

ISR(TIMER1_OVF_vect)
{
	tsl230_overflow_count++;
	onboard_led_on();
}

uint16_t tsl230_get(void)
{
	tsl230_sample_ready = 0;
	while(!tsl230_sample_ready); // block waiting
	return tsl230_sample;
}

/* When using this function, don't forget sei() */
void tsl230_init(void)
{
	/* set up our I/O */
	//setpin_out(DEBUG_DDR, DEBUG_PIN);
	setpin_out(S0_DDR, S0_PIN);
	setpin_out(S1_DDR, S1_PIN);
	setpin_out(S2_DDR, S2_PIN);
	setpin_out(S3_DDR, S3_PIN);
	setpin_in(FOUT_DDR, FOUT_PIN);

	/* set up timer/counter 1 to measure FOUT */
	timercounter1_setoverflow(OVERFLOW_TC1_33MS);
	//timercounter1_setoverflow(OVERFLOW_TC1_4MS); 

	/* use tc0 for sampling rate */
	timercounter0_setoverflow(OVERFLOW_TC0_16384US); // 16.384msec
	TIMSK0 |= _BV(TOIE0); // enable overflow interrupts

	tsl230_sample = 0;

	EICRA = _BV(ISC00); // interrupt on both edges
	EIMSK |= _BV(INT0); // enable int0 interrupts

	TIMSK1 |= _BV(TOIE1); // enable overflow interrupts
}

void tsl230_setsensitivity(enum tsl230_sensitivity sensitivity)
{
	switch (sensitivity)
	{
		case POWER_DOWN:
			printf("POWER_DOWN\n\r");
			clearpin(S1_PORT, S1_PIN);
			clearpin(S0_PORT, S0_PIN);
			break;
		case X1:
			printf("X1\n\r");
			clearpin(S1_PORT, S1_PIN);
			setpin(S0_PORT, S0_PIN);
			break;
		case X10:
			printf("X10\n\r");
			setpin(S1_PORT, S1_PIN);
			clearpin(S0_PORT, S0_PIN);
			break;
		case X100:
			printf("X100\n\r");
			setpin(S1_PORT, S1_PIN);
			setpin(S0_PORT, S0_PIN);
			break;
		default:
#ifdef DEBUG_PRINTF
			printf("tsl230_setsensitivity() invalid sensitivity\n\r");
#endif
			break;
	}
}

void tsl230_setscaling(enum tsl230_scaling scaling)
{
	switch (scaling)
	{
		case DIV_BY_1:
			printf("DIV_BY_1\n\r");
			clearpin(S3_PORT, S3_PIN);
			clearpin(S2_PORT, S2_PIN);
			break;
		case DIV_BY_2:
			printf("DIV_BY_2\n\r");
			clearpin(S3_PORT, S3_PIN);
			setpin(S2_PORT, S2_PIN);
			break;
		case DIV_BY_10:
			printf("DIV_BY_10\n\r");
			setpin(S3_PORT, S3_PIN);
			clearpin(S2_PORT, S2_PIN);
			break;
		case DIV_BY_100:
			printf("DIV_BY_100\n\r");
			setpin(S3_PORT, S3_PIN);
			setpin(S2_PORT, S2_PIN);
			break;
		default:
#ifdef DEBUG_PRINTF
			printf("tsl230_setscaling() invalid scale\n\r");
#endif
			break;
	}
}
