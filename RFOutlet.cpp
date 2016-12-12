#include <wiringPi.h>
#include "RFOutlet.h"
#include <stdio.h>

/* Timing from: https://aaroneiche.com/2016/01/31/weekend-project-wireless-outlet-control/ */

RFOutlet::RFOutlet(int pin){
	int result = wiringPiSetupGpio();
	if(result != 0){
		printf("wiringPi: %d\n",result);
	}

	this->pin = pin;
	longTime = 1800;
	shortTime = 600;

	pinMode(pin, OUTPUT);
	digitalWrite(pin, LOW);
}

void RFOutlet::send(uint8_t *message, int length) {
	for(int i=0; i<length; ++i) {
		uint8_t b=message[i];
		for(int j=0; j<8; ++j, b<<=1) {
			digitalWrite(pin, HIGH);
			delayMicroseconds((b&0x80) ? longTime : shortTime);
			digitalWrite(pin, LOW);
			delayMicroseconds((b&0x80) ? shortTime : longTime);
		}
	}
}

void RFOutlet::sendState(char channel, int outlet, bool state){
	uint8_t head, body, tail;
	
	head = 0b0110;

	if (outlet == 1){
		if (state)
			body = 0b10001000;
		else
			body = 0b10000100;
	}
	else if (outlet == 2){
		if (state)
			body = 0b10000010;
		else
			body = 0b10000001;
	}
	else if (outlet == 3){
		if (state)
			body = 0b10010000;
		else
			body = 0b10100000;
	}

	if (channel == 'F'){
		tail = 0b0000;
	}
	else if (channel == 'D'){
		tail = 0b0001;
	}

	uint8_t m[2];
	m[0] = (head << 4) | (body >> 4);
	m[1] = (body << 4) | (tail);

	for (int i=0; i<3; ++i) {
		send(m, 2);
		delay(10);
	}

	delay(10);
}

#if RFOUTLET_DEBUG

int main(int argc, char **argv) {
	RFOutlet outlet(26);

	char c;
	c = 'F';
	outlet.sendState(c,1,true);
	outlet.sendState(c,2,true);
	outlet.sendState(c,3,true);
	c = 'D';
	outlet.sendState(c,1,true);
	outlet.sendState(c,2,true);
	outlet.sendState(c,3,true);

	delay(500);

	c = 'F';
	outlet.sendState(c,1,false);
	outlet.sendState(c,2,false);
	outlet.sendState(c,3,false);
	c = 'D';
	outlet.sendState(c,1,false);
	outlet.sendState(c,2,false);
	outlet.sendState(c,3,false);

	delay(500);
}

#endif
