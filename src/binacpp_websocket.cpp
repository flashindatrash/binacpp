

#include "binacpp_websocket.h"

#include <utility>
#include "binacpp_logger.h"



struct lws_context *BinaCPP_websocket::context = NULL;
struct lws_protocols BinaCPP_websocket::protocols[] =
{
	{
		"example-protocol",
		BinaCPP_websocket::event_cb,
		0,
		65536,
	},
	{ NULL, NULL, 0, 0 } /* terminator */
};

map <struct lws *, BinaCPP_stream> BinaCPP_websocket::streams;



//--------------------------
int 
BinaCPP_websocket::event_cb( struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len )
{
    switch( reason )
	{
		case LWS_CALLBACK_CLIENT_ESTABLISHED: {
            lws_callback_on_writable(wsi);
            break;
        }
		case LWS_CALLBACK_CLIENT_RECEIVE:
		{

		    Json::Value json_result;
		    /* Handle incomming messages here. */
			try {
                auto it = streams.find(wsi);
                if (it != streams.end()) {
                    BinaCPP_stream& stream = it->second;

                    string str_result = string((char*)in);
                    Json::Reader reader;
                    reader.parse(str_result, json_result);

                    stream.callback(json_result);
                }
			} catch ( exception &e ) {
				BinaCPP_logger::write_log( "<BinaCPP_websocket::event_cb> \n%s", json_result.toStyledString().c_str() );
		 		BinaCPP_logger::write_log( "<BinaCPP_websocket::event_cb> Error ! %s", e.what() ); 
			}
			break;
		}
		case LWS_CALLBACK_CLIENT_WRITEABLE:
		{
			break;
		}

		case LWS_CALLBACK_CLOSED:
		case LWS_CALLBACK_CLIENT_CLOSED:
		case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
		{
            auto it = streams.find(wsi);
            if (it != streams.end()) {
                BinaCPP_stream& stream = it->second;

                Json::Value error;
                error["code"] = -1001;
                error["msg"] = "Connection closed" + (in == nullptr ? "" : " (" + string((char*)in) + ")");

                std::cout << "LWS_CALLBACK " << reason << std::endl;
                stream.callback(error);

                streams.erase(it);
            }
			break;
		}
		default:
			break;
	}

	return 0;
}


//-------------------
bool
BinaCPP_websocket::init( ) 
{
	struct lws_context_creation_info info;
	memset( &info, 0, sizeof(info) );

	info.port = CONTEXT_PORT_NO_LISTEN;
	info.protocols = protocols;
	info.gid = -1;
	info.uid = -1;
	info.options |= LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;

	context = lws_create_context( &info );
    return context != nullptr;
}


//----------------------------
// Register call backs
lws*
BinaCPP_websocket::connect_endpoint (
		CB fn,
		const char *path
	) 
{
	char ws_path[1024];
	strcpy( ws_path, path );
	
	
	/* Connect if we are not connected to the server. */
	struct lws_client_connect_info ccinfo = {0};
	ccinfo.context 	= context;
	ccinfo.address 	= BINANCE_WS_HOST;
	ccinfo.port 	= BINANCE_WS_PORT;
	ccinfo.path 	= ws_path;
	ccinfo.host 	= lws_canonical_hostname( context );
	ccinfo.origin 	= "origin";
	ccinfo.protocol = protocols[0].name;
	ccinfo.ssl_connection = LCCSCF_USE_SSL | LCCSCF_ALLOW_SELFSIGNED | LCCSCF_SKIP_SERVER_CERT_HOSTNAME_CHECK;

	struct lws* wsi = lws_client_connect_via_info(&ccinfo);
    if (wsi == nullptr)
        return nullptr;

    BinaCPP_stream stream;
    stream.callback = std::move(fn);

    streams[wsi] = stream;
    return wsi;
}

bool
BinaCPP_websocket::disconnect_endpoint (
        lws* wsi
)
{
    auto it = streams.find(wsi);
    if (it == streams.end())
        return false;

    streams.erase(it);
    return true;
}

//----------------------------
// Entering event loop
void 
BinaCPP_websocket::enter_event_loop() 
{
    while(not streams.empty())
	{	
		try {	
			lws_service( context, 0 );
		} catch ( exception &e ) {
		 	BinaCPP_logger::write_log( "<BinaCPP_websocket::enter_event_loop> Error ! %s", e.what() ); 
		 	break;
		}
	}
}

//----------------------------
// Exit event loop
void
BinaCPP_websocket::exit_event_loop ()
{
    streams.clear();
    lws_context_destroy(context);
}
