#include <string>
#include <sstream>
#include <vector>
#include <stdexcept>
#include <sys/time.h>
#include "mongoose.h"
#include "daemon.h"
#include "RFOutlet.h"

#define PORT "8000"

using namespace std;

RFOutlet *rf_outlet = NULL;

static vector<string> &split(const string &s, char delim, vector<string> &elems) {
	stringstream ss(s);
	string item;
	while (getline(ss, item, delim)) {
		elems.push_back(item);
	}
	return elems;
}

vector<string> split(const string &s, char delim) {
	vector<string> elems;
	split(s, delim, elems);
	return elems;
}

class http_exception:public runtime_error{
public:
	http_exception(int status_code,const char *what):status_code(status_code),runtime_error(what){}
	int status_code;
};

class http_response{
public:
	http_response(int status_code,const string &text):status_code(status_code),text(text){}
	int status_code;
	string text;
};

static http_response handle_request(http_message *hm){
	vector<string> tokens=split(string(hm->uri.p,hm->uri.len), '/');
	tokens.erase(tokens.begin());
	if (tokens.size()==3) {
		rf_outlet->sendState(tokens[0][0], atoi(tokens[1].c_str()), tokens[2]=="on");
		return http_response(200, "\"Command sent\"");
	}
	else{
		throw http_exception(404, "Endpoint not found");
	}
	return http_response(200, "");
}

static void ev_handler(struct mg_connection *nc, int ev, void *ev_data){
	struct http_message *hm = (struct http_message *) ev_data;

	switch (ev) {
		case MG_EV_HTTP_REQUEST:
			string status = "OK";
			http_response response(200, "");

			if (mg_vcmp(&hm->method, "OPTIONS") != 0){
				try{
					response = handle_request(hm);
				}
				catch(http_exception &e){
					response = http_response(e.status_code, e.what());
					status = e.what();
				}
			}

			mg_printf(nc, "HTTP/1.1 %d %s\r\n", response.status_code, status.c_str());
			mg_printf(nc, "Transfer-Encoding: chunked\r\n");
			mg_printf(nc, "Cache-Control: no-cache\r\n");
			if (mg_vcmp(&hm->method, "OPTIONS") == 0){
				mg_printf(nc, "Allow: HEAD,GET,OPTIONS\r\n");
				mg_printf(nc, "Access-Control-Allow-Origin: *\r\n");
				mg_printf(nc, "Access-Control-Allow-Headers: content-type\r\n");
			}
			else{
				mg_printf(nc, "Access-Control-Allow-Origin: *\r\n");
				mg_printf(nc, "Access-Control-Allow-Headers: content-type\r\n");
				mg_printf(nc, "Content-Type: application/json\r\n");
			}

			mg_printf(nc, "\r\n");

			mg_printf_http_chunk(nc, "%s", response.text.c_str());
			mg_send_http_chunk(nc, "", 0);
		break;
	}
}

class rest:public Daemon{
public:
	rest(int pin=0):Daemon("/tmp/remote.pid","/dev/null","/tmp/rfoutlet.log","/tmp/rfoutlet.err"),pin(pin){}

	void run(){
		rf_outlet=new RFOutlet(pin);

		mg_mgr_init(&mgr, NULL);

		nc = mg_bind(&mgr, PORT, ev_handler);
		if (nc == NULL) {
			fprintf(stderr, "Error starting server on port %s\n", PORT);
			exit(1);
		}

		mg_set_protocol_http_websocket(nc);

		printf("Starting RESTful server on port %s\n", PORT);
		for (;;) {
			mg_mgr_poll(&mgr, 100);
		}
		mg_mgr_free(&mgr);
	}

	int pin;
	struct mg_mgr mgr;
	struct mg_connection *nc;
};

int main(int argc,char **argv){
	if(argc>1 && strcmp(argv[1],"stop")==0){
		rest r;
		r.stop();
	}
	else if(argc>2 && strcmp(argv[1],"start")==0){
		rest r(atoi(argv[2]));
		r.start();
	}
	else if(argc>2 && strcmp(argv[1],"restart")==0){
		rest r(atoi(argv[2]));
		r.restart();
	}
	else if(argc>2 && strcmp(argv[1],"interactive")==0){
		rest r(atoi(argv[2]));
		r.run();
	}
	else{
		printf("%s [stop|start|restart]\n",argv[0]);
		return -1;
	}

	return 0;
}
