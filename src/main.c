#include <avr/io.h>
#include "SPI.h"
#include "timer.h"
#include <avr/interrupt.h>
#include <avr/eeprom.h>
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
	digipotInit();

	//comparator output pin as input
	DDRB &= ~_BV(PB1);

	MCUCR |= _BV(ISC01); //falling edge on INT0
	GIMSK |= _BV(INT0);
	digipotSet(0);
}

volatile uint8_t laserTripped = 0;

inline static void detectorArm() {
	laserTripped = 0;
}

uint8_t calibration = 0;

inline static void detectorCalibrate() {

	for(uint8_t b = 0; b < 255; b++) {
		digipotSet(b);
		delay(1);

		if(laserTripped) {
			if(b <= 255 - 10) {
				calibration = b + 10;
			} else {
				calibration = b;
			}
			digipotSet(calibration);
			break;
		}
	}
	detectorArm();
}

inline static uint16_t getBatteryVoltage() {
	ADCSRA |= _BV(ADSC);
	while(ADCSRA & _BV(ADSC)) {
		//wait
	}
	return ADC;
}

inline static void adcInit() {
	ADCSRA = _BV(ADEN) | _BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2);
	DIDR1 = _BV(ADC11D);
	ADMUXA = 11;
	ADMUXB = _BV(REFS1); //internal 2.2 reference
	delay(10);
	getBatteryVoltage();
	delay(10);
	getBatteryVoltage();
}


inline static void ledOn() {
	PORTB |= _BV(PB2);
}

inline static void ledOff() {
	PORTB &= ~_BV(PB2);
}

inline static uint8_t isBatteryLow() {
	return getBatteryVoltage() < 698;
}

inline static void ledRapidBlink(uint8_t times) {
	for(int i = 0; i < times; i++) {
		ledOn();
		delay(200);
		ledOff();
		delay(200);
	}
}

RFM69 radio;

static union {
	char buff[10];
	struct {
		uint8_t nodeId;
		uint8_t messageId;
		uint16_t batteryVoltage;
		uint8_t calibration;
		uint8_t laserTripped;
	} s;
} packet;

#define MESSAGE_STATUS 0
#define GATEWAY_ADDRESS 254

#define CMD_LASER_ON 1
#define CMD_LASER_OFF 2
#define CMD_CALIBRATE 3
#define CMD_GET_STATUS 4

void sendStatusPacket() {
	packet.s.messageId = MESSAGE_STATUS;
	packet.s.batteryVoltage = getBatteryVoltage();
	packet.s.calibration = calibration;
	packet.s.laserTripped = laserTripped;
	radio.send(GATEWAY_ADDRESS, packet.buff, 9);
}

#define I2C_ADDRESS_EEPROM_LOCATION (uint8_t*)0x00

inline void waitAndSendStatus() {
	for(uint8_t i = 0; i < packet.s.nodeId; i++) {
		delay(10);
	}
	sendStatusPacket();
}

int main() {
	sei();
	timerInit();

	ledInit();
	ledOn();
	delay(100);
	ledOff();

	packet.s.nodeId = eeprom_read_byte(I2C_ADDRESS_EEPROM_LOCATION);

	SPI.begin();
	laserInit();
	adcInit();

	detectorInit();
	detectorCalibrate();

	uint8_t radioInitialised = radio.initialize(RF69_868MHZ, packet.s.nodeId, 100);
	radio.encrypt(0);
	if(radioInitialised) {
		ledOn();
		delay(1500);
		sendStatusPacket();
		ledOff();
	} else {
		ledRapidBlink(2);
	}
	
	laserOn();

	while(1) {
		if(laserTripped) {
			ledOn();
			sendStatusPacket();
			delay(10);
			detectorArm();
		} else {
			ledOff();
		}

		if(radio.receiveDone()) {
			switch((char)radio.DATA[0]) {
				case CMD_LASER_ON: laserOn(); break;
				case CMD_LASER_OFF: laserOff(); break;
				case CMD_CALIBRATE: detectorCalibrate(); break;
				case CMD_GET_STATUS: waitAndSendStatus(); break;
			}
		}

		if(isBatteryLow()) {
			laserOff();
			radio.sleep();
			while(1) {
				ledRapidBlink(10);
				delay(2000);
			}
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

ISR(INT0_vect){
	laserTripped = 1;
}