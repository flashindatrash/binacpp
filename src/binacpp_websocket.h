


#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <exception>
#include <functional>

#include <json/json.h>
#include <libwebsockets.h>


#define BINANCE_WS_HOST "stream.binance.com"
#define BINANCE_WS_PORT 9443


using namespace std;

typedef std::function<int(Json::Value &)> CB;

class BinaCPP_websocket {


	static struct lws_context *context;
	static struct lws_protocols protocols[]; 

	static map <struct lws *,CB> handles ;
	
	public:
		static int  event_cb( struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len );
		static void connect_endpoint(
			CB user_cb,
			const char* path
		);
		static void init();
		static void enter_event_loop();
        static void exit_event_loop();


};
