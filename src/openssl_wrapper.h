#ifndef OPENSSL_WRAPPER_H
#define OPENSSL_WRAPPER_H

#include <string>
#include <memory>

#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include <src/log.h>

namespace sck {

//!Exception thrown from the SSL wrapper if it could not be started. 
class openssl_exception
	:public std::runtime_error {
		public:
		openssl_exception(const std::string& _e, int code)
			:std::runtime_error(
				std::string(_e
					+" [err "+std::to_string(code)+"]"
					+": "
					+ERR_error_string(code, NULL)
					)) {}
};

//!Wrapper around the context, to make sure it can be destroyed if 
//initialization of other parts fail in the openssl_wrapper constructor.
struct openssl_context {

						openssl_context(const std::string&, const std::string&, tools::log* =nullptr);
						~openssl_context();

	SSL_CTX *			context;
	tools::log *		log;
};

//!Wrapper for the SSL object, to ensure it can be destroyed if initialization
//!of other parts fail during the openssl_wrapper constructor.
struct openssl_ssl {

						openssl_ssl(SSL_CTX * _context, tools::log* =nullptr);
						~openssl_ssl();

	SSL *				ssl;
	tools::log *		log;
};

class openssl_wrapper {

	public:
						openssl_wrapper(const std::string&, const std::string&, tools::log* =nullptr);
						~openssl_wrapper();

	private:

	void				cleanup();

	std::unique_ptr<openssl_context>		context;
	std::unique_ptr<openssl_ssl>			ssl;
	tools::log *							log;
};

}

#endif