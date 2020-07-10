#pragma once

#include <string>
#include <memory>

#ifdef WITH_SSL

#include <openssl/ssl.h>

#endif

namespace sck {

struct ssl_client {

								ssl_client(int);
								~ssl_client();
		int						send(const std::string&);
		int						recv(char *, int);

#ifdef WITH_SSL
		SSL_CTX *				context=nullptr;
		SSL *					ssl=nullptr;
#endif
};

class client {

	public:

								//Builds the inet socket
								client(const std::string&, int, bool);

								//Builds the local socket
								client(const std::string&);

	void						send(const std::string&);
	std::string					receive(bool=false);

	private:

	int							file_descriptor=-1;
	std::unique_ptr<ssl_client> ssl_cl;
};

}
