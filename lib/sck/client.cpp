#include <sck/client.h>

#include <netdb.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>			//Memset...
#include <unistd.h>
#include <fcntl.h>

#ifdef WITH_SSL

#include <openssl/err.h>
#include <openssl/ssl.h>

#endif

#include <iostream>
#include <stdexcept>
#include <thread>

using namespace sck;

client::client(const std::string& _host, int _port, bool _secure)
	:ssl_cl(nullptr) {

	//Fill up the hints...
	addrinfo hints;
	memset(&hints, 0, sizeof hints);
	hints.ai_family=AF_UNSPEC;			//Will use either IPV4 or IPV6...
	hints.ai_socktype=SOCK_STREAM;		//For UDP.

	addrinfo *servinfo=nullptr;
	int getaddrinfo_res=getaddrinfo(_host.c_str(), std::to_string(_port).c_str(), &hints, &servinfo);
	if(0!=getaddrinfo_res) {
		throw std::runtime_error(gai_strerror(getaddrinfo_res));
	}

	//Connect to the first good result...
	addrinfo *p=servinfo;
	while(nullptr!=p) {

		file_descriptor=socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if(-1!=file_descriptor) {

			//Force adress reuse, in case a previous process is still hogging the port.
			int optval=1;
			if(-1==setsockopt(file_descriptor, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int))) {
				throw std::runtime_error("Unable to force address reuse");
			}

			if(0==connect(file_descriptor, p->ai_addr, p->ai_addrlen)) {

				if(_secure) {
					ssl_cl.reset(new ssl_client(file_descriptor));
				}
				break;
			}

			close(file_descriptor);
			file_descriptor=-1;
		}
		p=p->ai_next;
	}

	freeaddrinfo(servinfo);
	if(-1==file_descriptor) {
		throw std::runtime_error("Could not connect the socket! Perhaps the port is still in use?");
	}
}

client::client(const std::string& _local_sockfile) {

	file_descriptor=socket(AF_UNIX, SOCK_STREAM, 0);
	if(-1==file_descriptor) {

		throw std::runtime_error("Could not generate file descriptor for local socket");
	}

	sockaddr_un addr;
	memset(&addr, 0, sizeof(sockaddr_un));
	addr.sun_family=AF_UNIX;
	strcpy(addr.sun_path, _local_sockfile.c_str());


	if(0!=connect(file_descriptor, (sockaddr *)&addr, SUN_LEN(&addr))) {

		throw std::runtime_error("Could not connect to local socket");
	}
}

void client::send(const std::string& _msg) {

	int sent=nullptr==ssl_cl
		? ::send(file_descriptor, _msg.c_str(), _msg.size(), 0)
		: ssl_cl->send(_msg);

	if(0==sent) {
		throw std::runtime_error("send: server disconnected");
	}

	if(0 > sent) {
		throw std::runtime_error(strerror(errno));
	}
}


//Ok, public pls!!
std::string client::receive(bool _non_blocking) {

	auto set_non_blocking=[this]() {
		fcntl(file_descriptor, F_SETFL, fcntl(file_descriptor, F_GETFL, 0) | O_NONBLOCK);
	};

	auto set_blocking=[this]() {
		fcntl(file_descriptor, F_SETFL, fcntl(file_descriptor, F_GETFL, 0) & ~O_NONBLOCK);
	};

	if(_non_blocking) {
		set_non_blocking();
	}

	std::string result;

	const int bufsize=1024;
	char * buf=new char[bufsize];

	while(true) {

		memset(buf, 0, bufsize);

		int read=nullptr==ssl_cl
			? recv(file_descriptor, buf, bufsize, 0)
			: ssl_cl->recv(buf, bufsize);

		//The server disconnected us... This can happen anytime as we read the
		//input buffer, so let us make sure that we don't throw anything if
		//we don't want these messages discarded.

		//When a stream socket peer has performed an orderly shutdown, the return value will be 0 (the traditional "end-of-file" return).
		if(0==read) {
			file_descriptor=-1; //We use -1 again to signal disconnection.
			break;
		}

		//This is fun: we set the socket to nonblocking so any read operation
		//will set the error fflag EWOULDBLOCK or EAGAIN when there is nothing
		//!to read. If that is the case, we can just stop reading. Think of this
		//!as an alternative to using select statements.

		if(-1==read && _non_blocking) {

			if( (errno==EWOULDBLOCK) || (errno==EAGAIN)) {
				break;
			}

			set_blocking();
			throw std::runtime_error(strerror(errno));
		}

		result+=buf;
	}

	if(_non_blocking) {
		set_blocking();
	}

	delete [] buf;
	return result;
}

#ifndef WITH_SSL
ssl_client::ssl_client(int) {

	throw std::runtime_error("Client was compiled without SSL/TLS support");
}
#else

ssl_client::ssl_client(int _fd) {

std::cout<<"Creating new SSL client..."<<std::endl;

	SSL_load_error_strings();
	SSL_library_init();
	OpenSSL_add_all_algorithms();

#ifdef WITH_SSL_CURRENT
	context=SSL_CTX_new(TLS_method());
#else
	context=SSL_CTX_new(SSLv23_client_method());
#endif

	if(nullptr==context) {

		throw std::runtime_error(std::string("could not start openssl context: ")+ERR_error_string(ERR_get_error(), NULL));
	}

	SSL_CTX_set_options(context, SSL_OP_SINGLE_DH_USE);

	ssl=SSL_new(context);

	if(nullptr==ssl) {

		throw std::runtime_error(std::string("could not start openssl object: ")+ERR_error_string(ERR_get_error(), NULL));
	}

	if(1!=SSL_set_fd(ssl, _fd)) {
		throw std::runtime_error(std::string("could not set FD to SSL/TLS connect: ")+ERR_error_string(ERR_get_error(), NULL));
	}

	if(1!=SSL_connect(ssl)) {
		throw std::runtime_error(std::string("could not do SSL/TLS connect: ")+ERR_error_string(ERR_get_error(), NULL));
	}
}
#endif
//End of ssl version...

ssl_client::~ssl_client() {
#ifdef WITH_SSL

	if(nullptr!=context) {
		SSL_CTX_free(context);
	}

	if(nullptr!=ssl) {
		SSL_shutdown(ssl);
		SSL_free(ssl);
	}

	ERR_free_strings();
	EVP_cleanup(); //No effect in openssl 1.1.0.

//End of ssl version...
#endif
}



#ifdef WITH_SSL
int ssl_client::send(const std::string& _msg) {

	int sent=0;
	sent=SSL_write(ssl, _msg.c_str(), _msg.size());
	if(0 > sent) {

		throw std::runtime_error(std::string("ssl/tls write operation failed ")+ERR_error_string(ERR_get_error(), NULL));
	}
	return sent;
}
#else
int ssl_client::send(const std::string&) {
	return 0;
}
#endif

#ifdef WITH_SSL
int ssl_client::recv(char * _buf, int _len) {

	int read=0;

	read=SSL_read(ssl, _buf, _len);
	if(read < 0) {

		//Ok, this is ok, the socket is blocking...
		if(SSL_ERROR_WANT_READ==SSL_get_error(ssl, read)) {
			return read;
		}

		throw std::runtime_error(std::string("ssl/tls read operation failed :")+ERR_error_string(ERR_get_error(), NULL));
	}

	return read;
}
#else
int ssl_client::recv(char * , int ) {
	return 0;
}
#endif
