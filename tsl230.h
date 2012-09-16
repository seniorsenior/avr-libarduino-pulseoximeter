#ifndef TSL230_H
#define TSL230_H

#include <avr/io.h>
#include "libarduino.h"

/* connections */
#define FOUT_PORT	PORTD
#define FOUT_DDR	DDRD
#define FOUT_PIN	2
#define S0_PORT		PORTD
#define S0_DDR		DDRD
#define S0_PIN		4
#define S1_PORT		PORTD
#define S1_DDR		DDRD
#define S1_PIN		5
#define S2_PORT		PORTD
#define S2_DDR		DDRD
#define S2_PIN		6
#define S3_PORT		PORTD
#define S3_DDR		DDRD
#define S3_PIN		7

extern volatile uint32_t tsl230_overflow_count;

/* timer counter settings */

enum tsl230_sensitivity
{
	POWER_DOWN = 0,
	X1,
	X10,
	X100
};

enum tsl230_scaling
{
	DIV_BY_1 = 0,
	DIV_BY_2,
	DIV_BY_10,
	DIV_BY_100
};

void tsl230_init(void);
void tsl230_setsensitivity(enum tsl230_sensitivity sensitivity);
void tsl230_setscaling(enum tsl230_scaling scaling);
uint16_t tsl230_get(void);

#endif
