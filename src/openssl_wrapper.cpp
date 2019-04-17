#include "openssl_wrapper.h"

#include <src/log_tools.h>

using namespace sck;

#ifndef WITH_SSL

openssl_wrapper::openssl_wrapper(const std::string&, const std::string&, tools::log*) {

	throw openssl_exception();
}

//Finishes no SSL version
#else

openssl_wrapper::openssl_context::openssl_context(const std::string& _certpath, const std::string& _keypath, tools::log* _log)
	:context(nullptr), log(_log) {

#ifdef WITH_SSL_CURRENT
	if(log) {
		tools::info(*log)<<"openssl: context using TLS_method..."<<tools::endl();
	}

	context=SSL_CTX_new(TLS_method());
#else

	if(log) {
		tools::info(*log)<<"openssl: context using SSLv23_server_method..."<<tools::endl();
	}
	context=SSL_CTX_new(SSLv23_server_method());
#endif


	if(nullptr==context) {

		throw openssl_exception("could not start openssl context", ERR_get_error()); 
	}

	SSL_CTX_set_options(context, SSL_OP_SINGLE_DH_USE);

	if(log) {
		tools::info(*log)<<"openssl: context will use certificate file "<<_certpath<<" for cert..."<<tools::endl();
	}

	if(1!=SSL_CTX_use_certificate_file(context, _certpath.c_str(), SSL_FILETYPE_PEM)) {

		throw openssl_exception("could not use certificate file "+_certpath, ERR_get_error());
	}

	if(log) {
		tools::info(*log)<<"openssl: context use of certificate succeeded..."<<tools::endl();
	}

	if(log) {
		tools::info(*log)<<"openssl: context will use file "<<_keypath<<" for private key..."<<tools::endl();
	}

	if(1!=SSL_CTX_use_PrivateKey_file(context, _keypath.c_str(), SSL_FILETYPE_PEM)) {

		throw openssl_exception("could not use private key file "+_keypath, ERR_get_error());
	}

	if(log) {
		tools::info(*log)<<"openssl: context use of key succeeded..."<<tools::endl();
	}
}


int openssl_wrapper::recv(int _fd, char * _buffer, int _num) {

#ifndef WITH_SSL_CURRENT

	return 0;

#else

	if(1!=SSL_set_fd(ssl, _fd)) {
		throw openssl_exception("could not set fd", ERR_get_error());
	}

	int bytes_read=SSL_read(ssl, _buffer, num);

	if(0 > bytes_read) {
		throw openssl_exception("read operation failed", ERR_get_error());
	}

	return bytes_read;

#endif
}

void openssl_wrapper::accept() {

#ifdef WITH_SSL_CURRENT
	if(1!=SSL_set_fd(ssl, _fd)) {
		throw openssl_exception("could not set perform openssl accept procedure", ERR_get_error());
	}
#endif
}

openssl_wrapper::openssl_context::~openssl_context() {

	if(nullptr!=context) {
		
		if(log) {
			tools::info(*log)<<"openssl: context will be freed now..."<<tools::endl();
		}
		SSL_CTX_free(context);
	}

	context=nullptr;
}

openssl_wrapper::openssl_ssl::openssl_ssl(SSL_CTX * _context, tools::log* _log)
	:ssl(nullptr), log(_log) {

	ssl=SSL_new(_context);

	if(log) {
		tools::info(*log)<<"openssl: ssl object initialization succeeded..."<<tools::endl();
	}

	if(nullptr==ssl) {

		throw openssl_exception("could not start ssl object", ERR_get_error());
	}
}

openssl_wrapper::openssl_ssl::~openssl_ssl() {

	if(nullptr!=ssl) {

		if(log) {
			tools::info(*log)<<"openssl: ssl object will be freed now..."<<tools::endl();
		}
		SSL_shutdown(ssl);
		SSL_free(ssl);
	}

	ssl=nullptr;
}

openssl_wrapper::openssl_wrapper(const std::string& _certpath, const std::string& _keypath, tools::log * _log)
	:context(nullptr), ssl(nullptr), log(_log) {

	try {
		SSL_load_error_strings();
		SSL_library_init();
		OpenSSL_add_all_algorithms();

		context.reset(new openssl_context(_certpath, _keypath, _log));
		ssl.reset(new openssl_ssl(context.get()->context, _log));
	}
	catch(openssl_exception &e) {

		cleanup();
		throw;
	}
}

openssl_wrapper::~openssl_wrapper() {

	cleanup();	
}

void openssl_wrapper::cleanup() {

	ERR_free_strings();
	EVP_cleanup(); //No effect in openssl 1.1.0.
}

//Finishes SSL version.
#endif 