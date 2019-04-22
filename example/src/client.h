#ifndef EXAMPLE_CLIENT_H
#define EXAMPLE_CLIENT_H

#include <string>
#include <memory>

#ifdef WITH_SSL

#include <openssl/ssl.h>

#endif

namespace app {

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
								client(const std::string&, int, bool);
	void						send_message(const std::string&);								
	std::string					wait_for_answer();	

	private:

	void						send(const std::string&);
	std::string					receive(bool=false);
	int							file_descriptor=-1;

	std::unique_ptr<ssl_client> ssl_cl;
};

}

#endif