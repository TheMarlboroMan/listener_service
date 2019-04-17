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

	//TODO: What is this thing???
	//SSL_CTX_set_verify(context, SSL_VERIFY_PEER, NULL); 

	//TODO: 
	//SSL_CTX_load_verify_locations

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

	//TODO:
	//SSL_CTX_check_private_key

	if(log) {
		tools::info(*log)<<"openssl: context use of key succeeded..."<<tools::endl();
	}
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

int openssl_wrapper::recv(int _fd, char * _buffer, int _num) {

#ifndef WITH_SSL

	return 0;

#else

	if(1!=SSL_set_fd(ssl->ssl, _fd)) {
		throw openssl_exception("could not set fd", ERR_get_error());
	}

	int bytes_read=SSL_read(ssl->ssl, _buffer, _num);

	if(log) {
		tools::info(*log)<<"openssl: recv got "<<bytes_read<<" bytes and "<<_buffer<<tools::endl();
	}

	if(0 >= bytes_read) {

		std::string msg;

		switch(SSL_get_error(ssl->ssl, bytes_read)) {
			case SSL_ERROR_NONE: 				msg="ok"; break;
			case SSL_ERROR_ZERO_RETURN: 		msg="ZERO_RETURN"; break;
			case SSL_ERROR_WANT_READ: 			msg="WANT_READ"; break;
			case SSL_ERROR_WANT_WRITE: 			msg="WANT_WRITE"; break;
			case SSL_ERROR_WANT_CONNECT: 		msg="WANT_CONNECT"; break;
			case SSL_ERROR_WANT_ACCEPT: 		msg="WANT_ACCEPT"; break;
			case SSL_ERROR_WANT_X509_LOOKUP: 	msg="WANT_X509_LOOKUP"; break;
			case SSL_ERROR_SYSCALL:				msg="SYSCALL"; break;			
			case SSL_ERROR_SSL:					msg="SSL"; break;

		}

		throw openssl_exception("read operation failed: "+msg, ERR_get_error());
	}

	return bytes_read;

#endif
}

int openssl_wrapper::send(int _fd, const char * _buffer, int _num) {

#ifndef WITH_SSL

	return 0;

#else

	if(1!=SSL_set_fd(ssl->ssl, _fd)) {
		throw openssl_exception("could not set fd before write operation", ERR_get_error());
	}

	int bytes_written=SSL_write(ssl->ssl, _buffer, _num);

	if(0 > bytes_written) {
		throw openssl_exception("write operation failed", ERR_get_error());
	}

	return bytes_written;

#endif
}

void openssl_wrapper::accept(int _fd) {

#ifdef WITH_SSL

	if(1!=SSL_set_fd(ssl->ssl, _fd)) {
		throw openssl_exception("could not set perform openssl std_fd procedure before accept", ERR_get_error());
	}

	auto accept_res=SSL_accept(ssl->ssl);
	if(1!=accept_res) {
		throw openssl_exception("could not set perform openssl accept procedure", ERR_get_error());
	}
#endif
}

//Finishes SSL version.
#endif 
