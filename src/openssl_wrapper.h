#ifndef OPENSSL_WRAPPER_H
#define OPENSSL_WRAPPER_H

#include <string>
#include <memory>

#ifdef WITH_SSL

#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#endif

#include <src/log.h>

namespace sck {

//!Exception thrown from the SSL wrapper if it could not be started.  Can 
//!only be thrown from this module.
class openssl_exception
	:public std::runtime_error {
		private:

#ifndef WITH_SSL

	openssl_exception()
			:std::runtime_error("server was compiled without SSL support") {}

#else 

	openssl_exception(const std::string& _e, int code)
		:std::runtime_error(
			std::string(_e
				+" [err "+std::to_string(code)+"]"
				+": "
				+ERR_error_string(code, NULL)
				)) {}
#endif

		friend class openssl_context;
		friend class openssl_ssl;
		friend class openssl_wrapper;
};

class openssl_wrapper {



	public:

		 				openssl_wrapper(const std::string&, const std::string&, tools::log* =nullptr);

	int					recv(int, char *, int);
	void				accept();

#ifdef WITH_SSL

						~openssl_wrapper();

	private:

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

	void				cleanup();

	std::unique_ptr<openssl_context>		context;
	std::unique_ptr<openssl_ssl>			ssl;
	tools::log *							log;

//Finishes WITH_SSL version.
#endif
};



}

#endif