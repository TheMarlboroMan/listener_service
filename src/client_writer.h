#ifndef CLIENT_WRITER_H
#define CLIENT_WRITER_H

#include "connected_client.h"
#include "openssl_wrapper.h"

namespace sck {

class server;

//!Simple class to write data to a client.
class client_writer {

	public:

	//!Writes the given string to a client. A plain or secure underlying
	//!connection is automatically chosen based on the client and
	//!writer attributes.
	void 					write(const std::string&, const connected_client&);

	//!Returns true if the writer has SSL capabilities.
	bool					is_secure() const {return nullptr!=ssl_wrapper;}

	private:

	//Class constructor is private, this class can only be constructed from "server".
							client_writer(openssl_wrapper * = nullptr);


	//Implementation of unix-style send, wrapped with error handling.
	int						send(int, const std::string&, int, int=0);
	std::string				translate_error(int) const;

	//!This is, of course, non-owning and comes from the server.
	openssl_wrapper *		ssl_wrapper;

	friend class			server;
};

}

#endif
