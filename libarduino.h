#ifndef LIBARDUINO_H
#define LIBARDUINO_H

#include <stdio.h>
#include <inttypes.h>

#define UART_BAUD_RATE			57600 // default is 57600
#define UART_BAUD_SELECT		(F_CPU / (UART_BAUD_RATE * 16L) - 1)
#define ENABLE_SERIAL // serial port
//#define ENABLE_PWMSERVO // servo control (conflicts with regular pwm)
//#define ENABLE_PWM // motor or led control (conflicts with pwmservo)
//#define ENABLE_IR // infrared receiver
#define ENABLE_ADC // analog to digital convertor
#define ENABLE_TIMERCOUNTER // timer counter convenience functions

//#define ENABLE_ARDUINO_COMPAT // subset of arduino functions

//#define IR_DEBOUNCE // uncomment to debounce IR with a delay

/**************************************************************************
* no user configurable things to edit below this
**************************************************************************/

#define setpin_in(port, pin)	port &= ~_BV(pin)
#define setpin_out(port, pin)	port |= _BV(pin)
#define setpin(port, pin)		port |= _BV(pin)
#define clearpin(port, pin)		port &= ~_BV(pin)

void enable_onboard_led(void);
void onboard_led_on(void);
void onboard_led_off(void);

#ifdef ENABLE_ARDUINO_COMPAT // subset of arduino functions
enum pinmode {
	INPUT  = 0,
	OUTPUT = 1
};

enum pinstate {
	LOW = 0,
	HIGH = 1
};

void pinMode(uint8_t pin, enum pinmode mode);
void digitalWrite(uint8_t pin, enum pinstate value);
uint8_t digitalRead(uint8_t pin);
#ifdef ENABLE_PWM // using arduino compat and pwm
void analogWrite(uint8_t pin, uint8_t value);
#endif

#endif

#ifdef ENABLE_SERIAL
#define UART_BUFFER_SIZE	16
/* to use serial functions, do init then use printf() and getchar() */
void serial_init(void);
int serial_getchar(FILE* stream);
int serial_putchar(char data, FILE* stream);
#endif

#ifdef ENABLE_IR
#define IR_BUFFER_SIZE		16
void ir_init(void);
uint8_t ir_getcmd(void);
#endif

#ifdef ENABLE_PWMSERVO
void pwmservo_init(uint8_t pwmno);
void pwmservo_set(uint8_t servo, uint8_t pwmval); // pwmval 0-255
/* 16bit timer counter values for PWM */
#define SERVO_MIN_POS16		1400
#define SERVO_MID_POS16		3000
#define SERVO_MAX_POS16		4600
/* 8bit timer counter values for PWM */
#define SERVO_MIN_POS8		10
#define SERVO_MID_POS8		25
#define SERVO_MAX_POS8		40
#endif

#ifdef ENABLE_PWM
void pwm_init(uint8_t pwmnum);
void pwm_set(uint8_t pwmnum, uint8_t pwmval); // pwmval 0-255
#endif

#ifdef ENABLE_ADC
void adc_init(void);
uint16_t adc_getval(uint8_t adcnum);
#endif

#ifdef ENABLE_TIMERCOUNTER
/* csbits - timer counter 1 */
#define DISABLE_CLOCK			0
#define OVERFLOW_16BIT_4MS		1 // 4 msec
#define OVERFLOW_16BIT_32MS		2 // 32.768 msec
#define OVERFLOW_16BIT_262MS	3 // 262.144 msec
#define OVERFLOW_16BIT_1048MS	4 // 1.048576 sec
#define OVERFLOW_16BIT_4194MS	5 // 4.1924304 sec
void timercounter1_setclock(uint8_t csbits);

#endif

#if	defined(__AVR_ATmega168__) || \
 	defined(__AVR_ATmega328P__)

#define UART0_DATA	UDR0

#define UART0_ISR_VECT USART_RX_vect

#else
/* unsupported type */
#error "Processor type not supported in libarduino.c !"
#endif

#if !((F_CPU == 16000000) || (F_CPU == 8000000))
#error "Processor speed not supported in libarduino.c !"
#endif

#if (F_CPU == 8000000)
#warn "Processor speed only partically supported by libarduino.c !"
#endif

#endif
