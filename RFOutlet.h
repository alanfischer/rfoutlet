#ifndef RFOUTLET_H
#define RFOUTLET_H

#include <stdint.h>

class RFOutlet{
public:
	RFOutlet(int pin);

	typedef enum{
		unknown_product = 0,
		tr016_rev02 = 2,
		tr016_rev03 = 3,
	} product_t;

	static product_t parseProduct(const char* product);
	static bool parseState(const char* state);

	void sendState(product_t product, char channel, int outlet, bool state);
	void send(int shortTime, int longTime, uint8_t *message, int length);

protected:
	int pin;
	int repeat;
	int repeatDelayScaler;
	int longRepeat;
	int longRepeatDelayScaler;
};

#endif
