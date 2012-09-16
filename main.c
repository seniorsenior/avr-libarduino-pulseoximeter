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
#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "libarduino.h"
#include "timercountercalcs.h"

volatile uint8_t sample_overflow = 0;
volatile uint8_t take_sample = 0;

#define DEBUG_PORT	PORTC
#define DEBUG_DDR	DDRC
#define DEBUG_PIN	2

ISR(TIMER0_OVF_vect)
{
	sample_overflow++;

	if (!(sample_overflow % 4))
	{
		take_sample = 1;
	}
}

int main (void)
{
	uint16_t c = 0;

	enable_onboard_led();
	setpin_out(DEBUG_DDR, DEBUG_PIN);
	serial_init();
	fdevopen(serial_putchar, serial_getchar);

	adc_init();

	/* use tc0 for sampling rate */
	timercounter0_setoverflow(OVERFLOW_TC0_16384US); // 16.384msec
	TIMSK0 |= _BV(TOIE0); // enable overflow interrupts

	sei(); // enable interrupts

	while (1)
	{
		if (take_sample)
		{
			onboard_led_on();
			setpin(DEBUG_PORT, DEBUG_PIN);
			c = adc_getval(0); // take sample
			printf("r=%d\n\r", c);

			clearpin(DEBUG_PORT, DEBUG_PIN);
			onboard_led_off();

			take_sample = 0;
		}
	}

	return 1;
}

