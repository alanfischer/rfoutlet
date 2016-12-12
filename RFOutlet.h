#ifndef RFOUTLET_H
#define RFOUTLET_H

#include <stdint.h>

class RFOutlet{
public:
	RFOutlet(int pin);

	void sendState(char channel, int outlet, bool state);
	void send(uint8_t *message, int length);

protected:
	int pin;
	int longTime;
	int shortTime;
	int tries;
};

#endif
