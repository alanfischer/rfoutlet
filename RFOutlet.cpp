#include <wiringPi.h>
#include "RFOutlet.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

/* Author: Alan Fischer <alan@lightningtoads.com> 12/11/16 */
/* Rev 2 timing from: https://aaroneiche.com/2016/01/31/weekend-project-wireless-outlet-control/ */

RFOutlet::RFOutlet(int pin){
	int result = wiringPiSetupGpio();
	if(result != 0){
		printf("wiringPi: %d\n",result);
	}

	this->pin = pin;
	repeat = 4;
	repeatDelayScaler = 5;
	longRepeat = 2;
	longRepeatDelayScaler = 50;

	pinMode(pin, OUTPUT);
	digitalWrite(pin, LOW);
}

RFOutlet::product_t RFOutlet::parseProduct(const char* product) {
	if(strstr(product, "2")!=0) {
		return tr016_rev02;
	}
	else if(strstr(product, "3")!=0) {
		return tr016_rev03;
	}
	return unknown_product;
}

bool RFOutlet::parseState(const char* state) {
	return strlen(state)>=2 && tolower(state[0]) == 'o' &&  tolower(state[1]) == 'n';
}

void RFOutlet::send(int shortTime, int longTime, uint8_t *message, int length) {
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

void RFOutlet::sendState(product_t product, char channel, int outlet, bool state){
	uint8_t head, body, tail;
	int shortTime=0, longTime=0;

	switch (product) {
		case tr016_rev02:
			shortTime = 600;
		break;
		case tr016_rev03:
			shortTime = 185;
		break;
	}
	longTime = shortTime * 3;
	
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
	else if (channel == 'C'){
		tail = 0b0010;
	}

	uint8_t m[2];
	m[0] = (head << 4) | (body >> 4);
	m[1] = (body << 4) | (tail);
	
	for (int l=0; l<longRepeat; ++l) {
		if (l > 0) {
			delayMicroseconds((shortTime + longTime) * longRepeatDelayScaler);
		}

		for (int r=0; r<repeat; ++r) {
			if (r > 0) {
				delayMicroseconds((shortTime + longTime) * repeatDelayScaler);
			}

			send(shortTime, longTime, m, 2);
		}
	}
}

#if RFOUTLET_MAIN

int main(int argc, char **argv) {
	if (argc < 6) {
		printf("%s [pin] [product] [channel] [number] [on/off]\n", argv[0]);
		return 1;
	}
	RFOutlet outlet(atoi(argv[1]));
	outlet.sendState(RFOutlet::parseProduct(argv[2]), argv[3][0], atoi(argv[4]), RFOutlet::parseState(argv[5]));
}

#endif
