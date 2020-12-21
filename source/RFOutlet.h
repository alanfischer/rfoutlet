#ifndef RFOUTLET_H
#define RFOUTLET_H

#include <string>
#include <vector>
#include <deque>
#include <stdint.h>
#include <pthread.h>

class RFOutlet {
public:
	RFOutlet(int pin315, int pin433);
	~RFOutlet();

	typedef enum {
		unknown_product = 0,
		tr016_rev02 = 2,
		tr016_rev03 = 3,
		tr016_rev04 = 4,
		max_products = 5,
	} product_t;

	typedef enum {
		max_channels = 6
	} channel_t;

	typedef enum {
		max_outlets = 3
	} outlet_t;

	typedef struct {
		product_t product;
		channel_t channel;
		outlet_t outlet;
		bool state;
	} device_t;

	static product_t parseProduct(const std::string& product);
	static bool parseState(const std::string& state);

	void setState(product_t product, const char* channel, int outlet, bool state);
	bool getState(product_t product, const char* channel, int outlet);
	bool isBusy();
	void delay(int microseconds);

	static void logf(const char* format,...);
	static void setLog(void (*cb)(const char*));

protected:
	static void* start(void *self);
	void run();
	std::string setupPin(int pin);
	void closePin(int pin);
	device_t *find(product_t product, const char *channel, int outlet);
	void sendState(product_t product, char channel, int outlet, bool state);
	void send(int pin, int shortTime, int longTime, uint8_t *message, int length);
	void write(int pin, bool value);

	int pin315, pin433;
	std::string valuefilename315, valuefilename433;
	int repeat;
	int repeatDelayScaler;
	int longRepeat;
	int longRepeatDelayScaler;
	pthread_t thread;
	pthread_mutex_t mutex;
	std::vector<device_t*> devices;
	std::deque<device_t*> updatedDevices;
	bool running;
	std::vector<device_t*>::iterator current;

	static void (*log)(const char*);
};

#endif
