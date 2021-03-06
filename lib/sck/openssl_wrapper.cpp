#include <sck/openssl_wrapper.h>
#include <sck/exception.h>

#include <lm/logger.h>

using namespace sck;

#ifndef WITH_SSL

openssl_wrapper::openssl_wrapper(const std::string&, const std::string&, lm::logger*) {

	throw openssl_exception();
}

int openssl_wrapper::recv(int, char *, int) {

	return 0;
}

int openssl_wrapper::send(int, const char *, int) {

	return 0;
}

void openssl_wrapper::accept(int) {

}


//Finishes no SSL version
#else

openssl_wrapper::openssl_context::openssl_context(const std::string& _certpath, const std::string& _keypath, lm::logger * _log)
	:context(nullptr), log(_log) {

#ifdef WITH_SSL_CURRENT
	if(log) {
		lm::log(*_logger, lm::lvl::info)<<"openssl: context using TLS_method..."<<std::endl;
	}

	context=SSL_CTX_new(TLS_method());
#else

	if(log) {
		lm::log(*_logger, lm::lvl::info)<<<<"openssl: context using SSLv23_server_method..."<<std::endl;
	}
	context=SSL_CTX_new(SSLv23_server_method());
#endif

	if(nullptr==context) {

		throw openssl_exception("could not start openssl context", ERR_get_error());
	}

	//Always create a new key when using temporary/ephemeral DH parameter
	SSL_CTX_set_options(context, SSL_OP_SINGLE_DH_USE);

	//TODO: What is this thing???
	//SSL_CTX_set_verify(context, SSL_VERIFY_PEER, NULL);

	//TODO:
	//SSL_CTX_load_verify_locations

	if(log) {
		lm::log(*_logger, lm::lvl::info)<<"openssl: context will use certificate file "<<_certpath<<" for cert..."<<std::endl;
	}

	if(1!=SSL_CTX_use_certificate_file(context, _certpath.c_str(), SSL_FILETYPE_PEM)) {

		throw openssl_exception("could not use certificate file "+_certpath, ERR_get_error());
	}

	if(log) {
		lm::log(*_logger, lm::lvl::info)<<<<"openssl: context use of certificate succeeded..."<<std::endl;
	}

	if(log) {
		lm::log(*_logger, lm::lvl::info)<<"openssl: context will use file "<<_keypath<<" for private key..."<<std::endl;
	}

	if(1!=SSL_CTX_use_PrivateKey_file(context, _keypath.c_str(), SSL_FILETYPE_PEM)) {

		throw openssl_exception("could not use private key file "+_keypath, ERR_get_error());
	}

	//TODO:
	//SSL_CTX_check_private_key

	if(log) {
		lm::log(*_logger, lm::lvl::info)<<<<"openssl: context use of key succeeded..."<<std::endl;
	}
}

openssl_wrapper::openssl_context::~openssl_context() {

	if(nullptr!=context) {

		if(log) {
			lm::log(*_logger, lm::lvl::info)<<"openssl: context will be freed now..."<<std::endl;
		}
		SSL_CTX_free(context);
	}

	context=nullptr;
}

openssl_wrapper::openssl_ssl::openssl_ssl(SSL_CTX * _context, lm::logger* _log)
	:ssl(nullptr), log(_log) {

	ssl=SSL_new(_context);

	if(log) {
		lm::log(*_logger, lm::lvl::info)<<"openssl: ssl object initialization succeeded..."<<std::endl;
	}

	if(nullptr==ssl) {

		throw openssl_exception("could not start ssl object", ERR_get_error());
	}
}

openssl_wrapper::openssl_ssl::~openssl_ssl() {

	if(nullptr!=ssl) {

		if(log) {
			lm::log(*_logger, lm::lvl::info)<<"openssl: ssl object will be freed now..."<<std::endl;
		}
		SSL_shutdown(ssl);
		SSL_free(ssl);
	}

	ssl=nullptr;
}

openssl_wrapper::openssl_wrapper(const std::string& _certpath, const std::string& _keypath, lm::logger * _log)
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

	if(1!=SSL_set_fd(ssl->ssl, _fd)) {
		throw read_exception(std::string("could not set fd: ")+ERR_error_string(ERR_get_error(), NULL), true);
	}

	int bytes_read=SSL_read(ssl->ssl, _buffer, _num);

	if(log) {
		lm::log(*_logger, lm::lvl::info)<<"openssl: recv got "<<bytes_read<<" bytes and "<<_buffer<<std::endl
	}

	if(0 >= bytes_read) {

/*
		std::string msg="SSL recv error: ";
		switch(SSL_get_error(ssl->ssl, bytes_read)) {
			case SSL_ERROR_NONE: 				msg+="ok"; break;
			case SSL_ERROR_ZERO_RETURN: 		msg+="ZERO_RETURN"; break;
			case SSL_ERROR_WANT_READ: 			msg+="WANT_READ"; break;
			case SSL_ERROR_WANT_WRITE: 			msg+="WANT_WRITE"; break;
			case SSL_ERROR_WANT_CONNECT: 		msg+="WANT_CONNECT"; break;
			case SSL_ERROR_WANT_ACCEPT: 		msg+="WANT_ACCEPT"; break;
			case SSL_ERROR_WANT_X509_LOOKUP: 	msg+="WANT_X509_LOOKUP"; break;
			case SSL_ERROR_SYSCALL:				msg+="SYSCALL"; break;
			case SSL_ERROR_SSL:					msg+="SSL"; break;

		}

		throw read_exception(msg+": "+ERR_error_string(ERR_get_error(), NULL), true);
*/
		throw read_exception(ERR_error_string(ERR_get_error(), NULL), true);
	}

	return bytes_read;
}

int openssl_wrapper::send(int _fd, const char * _buffer, int _num) {

	if(log) {
lm::log(*_logger, lm::lvl::debug)<<"sending '"<<_buffer<<"' ("<<_num<<") to "<<_fd<<std::endl;
	}

	if(1!=SSL_set_fd(ssl->ssl, _fd)) {
		throw send_exception(std::string("could not set fd before write operation: ")+ERR_error_string(ERR_get_error(), NULL), true);
	}

	int bytes_written=SSL_write(ssl->ssl, _buffer, _num);

	if(0 > bytes_written) {
		throw send_exception(std::string("write operation failed ")+ERR_error_string(ERR_get_error(), NULL), true);
	}

	return bytes_written;
}

void openssl_wrapper::accept(int _fd) {

	if(1!=SSL_set_fd(ssl->ssl, _fd)) {

		throw openssl_exception("could not perform openssl std_fd procedure before accept", ERR_get_error());
	}

	if(1!=SSL_accept(ssl->ssl)) {
		throw openssl_exception("could not perform openssl accept procedure", ERR_get_error());
	}
}

//Finishes SSL version.
#endif
