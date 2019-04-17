#ifndef CLIENT_WRITER_H
#define CLIENT_WRITER_H

#include "connected_client.h"
#include "openssl_wrapper.h"

namespace sck {

//!Simple class to write data to a client.
class client_writer {

	public:

							client_writer(openssl_wrapper * = nullptr);

	//!Writes the given string to a client.
	void 					write(const std::string&, const connected_client&);
	bool					has_ssl() const {return nullptr!=ssl_wrapper;} 

	private:

	//!This is, of course, non-owning and comes from the server.
	openssl_wrapper *		ssl_wrapper;		
};

}

#endif
