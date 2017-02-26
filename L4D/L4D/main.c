/*
 * L4D.c
 *
 * Created: 10/15/2016 10:46:29 AM
 * Author : wudao
 */ 

#include <avr/io.h>
#include "m_general.h"
#include "m_bus.h"
#include "m_rf.h"

#define CHANNEL 1
#define RXADDRESS 0x3E
#define PACKET_LENGTH 3

char buffer[PACKET_LENGTH] = {0,0,0}; // buffer initialization
volatile double frequency = 0;
volatile double duration = 0;
volatile int t = 0;
volatile int flag = 0;

void init()
{
    m_clockdivide(0); //the system clock must have 16 MHz frequency to use mBUS

	clear(TCCR1B,CS12);
	clear(TCCR1B,CS11);
	set(TCCR1B,CS10);
	// set the timer1 prescaler to /1
	set(TCCR1B,WGM13);
	set(TCCR1B,WGM12);
	set(TCCR1A,WGM11);
	set(TCCR1A,WGM10);
	// set the timer1 modes to (mode 15) UP to OCR1A, PWM mode
	set(TCCR1A,COM1C1);
	clear(TCCR1A,COM1C0);
	// set OC1C (B7 output) options to clear at OCR1C, set at rollover

	set(TCCR3B,CS32);
	clear(TCCR3B,CS31);
	set(TCCR3B,CS30);
	// set the timer3 prescaler to /1024
	clear(TCCR3B,WGM33);
	set(TCCR3B,WGM32);
	clear(TCCR3A,WGM31);
	clear(TCCR3A,WGM30);
	// set the timer3 modes to (mode 4) UP to OCR3A

	set(TIMSK1,OCIE1A);
	set(TIMSK3,OCIE3A);
	set(EIMSK,INT2);
	sei();
	// set and enable timer1&3 and mRF interrupt
}

int main(void)
{
    init();
	m_bus_init();// enable mBUS
	m_rf_open(CHANNEL,RXADDRESS,PACKET_LENGTH); // configure mRF
	while(1)
	{
	    if (flag == 1)
		{
		OCR1A = 200;
		OCR1C = OCR1A*(1+sin(t*2*3.1415926*frequency*200/16000000))/2;
		set (DDRB,7);
		// enable B7 output
		flag = 0;
		}
	}
}

ISR(INT2_vect)
{
	m_rf_read(buffer,PACKET_LENGTH); // pull the packet
	m_red(TOGGLE);
	frequency = 0.1 * (buffer[0] + 256 * buffer[1]); // calculate frequency from FL and FH
	duration = 0.01 * buffer[2]; // calculate duration from DD
	OCR3A = 16000000*duration/1024; // 16 MHz system frequency / timer3 prescaler
	TCNT3 = 0; // reset timer3
	flag = 1;
}

ISR(TIMER1_COMPA_vect){t++;}

ISR(TIMER3_COMPA_vect){clear (DDRB,7); m_green(TOGGLE);}