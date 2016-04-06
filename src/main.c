#include <avr/io.h>
#include "SPI.h"
#include "timer.h"
#include <avr/interrupt.h>
#include "RFM69.h"

inline static void digipotDeselect() {
	PORTA |= _BV(PA1);
}

inline static void digipotInit() {
	DDRA |= _BV(PA1);
	digipotDeselect();
}

inline static void digipotSelect() {
	PORTA &= ~_BV(PA1);
}

inline static void ledInit() {
	DDRB |= _BV(PB2);
	PORTB &= ~_BV(PB2);
}

inline static void digipotSet(uint8_t b) {
	digipotSelect();
	SPI.transfer(0x11);
	SPI.transfer(b);
	digipotDeselect();
}


inline static void laserOn() {
	PORTA |= _BV(PA7);
}

inline static void laserOff() {
	PORTA &= ~_BV(PA7);
}

inline static void laserInit() {
	DDRA |= _BV(PA7);
	laserOff();
}


inline static void detectorInit() {
	//laser detector as input
	DDRB &= ~_BV(PB1);
}

inline static void adcInit() {
	ADCSRA = _BV(ADEN) | _BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2);
	DIDR1 = _BV(ADC11D);
	ADMUXA = 11;
	ADMUXB = _BV(REFS1); //internal 2.2 reference
}

inline static uint16_t getBatteryVoltage() {
	ADCSRA |= _BV(ADSC);
	while(ADCSRA & _BV(ADSC)) {
		//wait
	}
	return ADC;
}

inline static void ledOn() {
	PORTB |= _BV(PB2);
}

inline static void ledOff() {
	PORTB &= ~_BV(PB2);
}

//RFM69 radio;
char *buff = "winrar 42";

int main() {
	sei();
	laserInit();
	timerInit();
	ledInit();
	adcInit();
	while(1) {
		if(getBatteryVoltage() < 698) {
			ledOff();
		} else {
			ledOn();
		}
	}
}


/*
int main2() {
	sei();
	timerInit();
	SPI.begin();

	ledInit();
	digipotInit();
	laserInit();


	uint8_t b = 0;

	uint8_t radioInitialised = radio.initialize(RF69_868MHZ, 98, 100);
	if(radioInitialised) {
		laserOn();
		delay(1000);
		laserOff();
	} else {
		laserOn();
		delay(200);
		laserOff();
		delay(200);
		laserOn();
		delay(200);
		laserOff();
	}
	radio.encrypt(0);

	while(1) {
		if(PINB & _BV(PINB1)) {
			PORTB |= _BV(PB2);
		} else {
			PORTB &= ~_BV(PB2);
		}
		delay(10);
		digipotSet(b++);
		radio.send(1, buff, 9);
	}
	return 0;
}
*/