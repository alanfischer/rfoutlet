#include <wiringPi.h>
#include "RFOutlet.h"
#include <stdlib.h>
#include <string.h>
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

#if RFOUTLET_MAIN

int main(int argc, char **argv) {
	if (argc < 5) {
		printf("%s [pin] [channel] [number] [on/off]\n", argv[0]);
		return 1;
	}

	RFOutlet outlet(atoi(argv[1]));

	outlet.sendState(argv[2][0], atoi(argv[3]), strcmp("on", argv[4])==0);
}

#endif
