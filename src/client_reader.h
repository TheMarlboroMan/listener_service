#ifndef CLIENT_READER_H
#define CLIENT_READER_H

#include <string>

#include "connected_client.h"
#include "openssl_wrapper.h"


namespace sck {

class client_reader {

	public:
							client_reader(int, openssl_wrapper * = nullptr);
							~client_reader();

	//!Reads from a client. A plain or secure underlying connection is 
	//!automatically chosen based on the client and reader attributes.
	//!In the first read, it will change the verified status of the client.
	std::string				read(connected_client&, bool=false);

	//!Returns true if the reader has SSL capabilities.
	bool					is_secure() const {return nullptr!=ssl_wrapper;} 

	private:

	const int 				read_message_buffer_size;
	char * 					read_message_buffer=nullptr;

	//!This is, of course, non-owning and comes from the server.
	openssl_wrapper *		ssl_wrapper;

	void					set_client_security(connected_client&);

};

}
#endif
