#include "RFOutlet.h"
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <stdarg.h>

using namespace std;

/* Author: Alan Fischer <alan@lightningtoads.com> 12/11/16 */
/* Rev 2 timing from: https://aaroneiche.com/2016/01/31/weekend-project-wireless-outlet-control/ */

void (*RFOutlet::log)(const char*) = NULL;

void RFOutlet::setLog(void (*cb)(const char*)){
	log = cb;
}

void RFOutlet::logf(const char* format, ...){
	char data[1024];
	va_list argptr;
	va_start(argptr, format);
	vsprintf(data, format, argptr);
	va_end(argptr);
	if(log){
		log(data);
	}
	else{
		printf("%s\n",data);
	}
}

int stateIndex(RFOutlet::product_t product, const char* channel, int outlet) {
	return (product * (RFOutlet::max_channels * RFOutlet::max_outlets)) + ((channel[0] - 'A') * RFOutlet::max_outlets) + outlet;
}

RFOutlet::RFOutlet(int pin):
	pin(pin),
	repeat(4),
	repeatDelayScaler(5),
	longRepeat(2),
	longRepeatDelayScaler(50)
{
	memset(states,0,sizeof(states));

	ofstream exportfile("/sys/class/gpio/export");
	if(!exportfile){
		logf("Unable to export pin!");
		return;
	}
	exportfile << pin;
	exportfile.close();

	// Delay for the pin to export
	delay(500000);

	stringstream filename;
	filename << "/sys/class/gpio/gpio" << pin;

	ofstream directionfile((filename.str() + "/direction").c_str());
	if(!directionfile){
		logf("Unable to set pin direction!");
		return;
	}
	directionfile << "out";
	directionfile.close();

	valuefilename = filename.str() + "/value";

	write(pin, false);
}

RFOutlet::~RFOutlet(){
	ofstream unexportfile("/sys/class/gpio/unexport");
	unexportfile << pin;
	unexportfile.close();
}

RFOutlet::product_t RFOutlet::parseProduct(const string& product) {
	if(product.find("2")!=string::npos) {
		return tr016_rev02;
	}
	else if(product.find("3")!=string::npos) {
		return tr016_rev03;
	}
	return unknown_product;
}

bool RFOutlet::parseState(const string& state) {
	string istate = state;
	transform(istate.begin(),istate.end(),istate.begin(),::tolower);
	return istate == "on" || istate == "true";
}

void RFOutlet::sendState(product_t product, const char *channel, int outlet, bool state){
	uint8_t head=0, body=0, tail=0;
	int shortTime=0, longTime=0;

	switch (product) {
		case tr016_rev02:
			shortTime = 600;
		break;
		case tr016_rev03:
			shortTime = 200;
		break;
		default:
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

	if (channel[0] == 'F'){
		tail = 0b0000;
	}
	else if (channel[0] == 'D'){
		tail = 0b0001;
	}
	else if (channel[0] == 'C'){
		tail = 0b0010;
	}

	uint8_t m[2];
	m[0] = (head << 4) | (body >> 4);
	m[1] = (body << 4) | (tail);
	
	for (int l=0; l<longRepeat; ++l) {
		if (l > 0) {
			delay((shortTime + longTime) * longRepeatDelayScaler);
		}

		for (int r=0; r<repeat; ++r) {
			if (r > 0) {
				delay((shortTime + longTime) * repeatDelayScaler);
			}

			send(shortTime, longTime, m, 2);
		}
	}

	states[stateIndex(product,channel,outlet)] = state;
}

bool RFOutlet::getState(product_t product, const char *channel, int outlet){
	return states[stateIndex(product,channel,outlet)];
}

void RFOutlet::send(int shortTime, int longTime, uint8_t *message, int length) {
	for(int i=0; i<length; ++i) {
		uint8_t b=message[i];
		for(int j=0; j<8; ++j, b<<=1) {
			write(pin, true);
			delay((b&0x80) ? longTime : shortTime);
			write(pin, false);
			delay((b&0x80) ? shortTime : longTime);
		}
	}
}

void RFOutlet::write(int pin, bool value) {
	ofstream valuefile(valuefilename.c_str());
	valuefile << (value ? "1" : "0");
	valuefile.close();
}

void RFOutlet::delay(int microseconds) {
	struct timespec tv;
	tv.tv_sec = microseconds / 1000000,
	tv.tv_nsec = (microseconds - tv.tv_sec * 1000000) * 1000;
	nanosleep(&tv,NULL);
}

extern "C" {

RFOutlet::product_t RFOutlet_parseProduct(const char* product){return RFOutlet::parseProduct(product);}
bool RFOutlet_parseState(const char* state){return RFOutlet::parseState(state);}

RFOutlet *RFOutlet_new(int pin){return new RFOutlet(pin);}
void RFOutlet_delete(RFOutlet *rfoutlet){delete rfoutlet;}

void RFOutlet_sendState(RFOutlet *rfoutlet, RFOutlet::product_t product, const char* channel, int outlet, bool state){rfoutlet->sendState(product,channel,outlet,state);}
bool RFOutlet_getState(RFOutlet *rfoutlet, RFOutlet::product_t product, const char* channel, int outlet){return rfoutlet->getState(product,channel,outlet);}

void RFOutlet_setLog(void (*cb)(const char*)){RFOutlet::setLog(cb);}

}

#if RFOUTLET_MAIN

int main(int argc, char **argv) {
	if (argc < 6) {
		printf("%s [pin] [product] [channel] [number] [on/off]\n", argv[0]);
		return 1;
	}
	RFOutlet outlet(atoi(argv[1]));
	outlet.sendState(RFOutlet::parseProduct(argv[2]), argv[3], atoi(argv[4]), RFOutlet::parseState(argv[5]));
}

#endif
