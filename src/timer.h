#ifndef _TIMER_H_INCLUDED
#define _TIMER_H_INCLUDED

#include <avr/io.h>

extern "C" {

unsigned long millis();
unsigned long micros();
void delay(unsigned long ms);
inline void timerInit() {
	TIMSK0 = _BV(TOIE0);
	TCCR0B = _BV(CS00) | _BV(CS01);
}

}
#endif