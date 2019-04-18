#ifndef LISTENER_SERVICE_EXCEPTION_H
#define LISTENER_SERVICE_EXCEPTION_H

#include <stdexcept>
#include <string>

namespace sck {

//!Internal exception thrown when a read comes in and fails.
class client_disconnected_exception
	:public std::runtime_error {
	public:
		client_disconnected_exception()
			:std::runtime_error("Client disconnected!") {

	}
};

//!Exception that denotes a client using a secure connection trying to contact
//!a server that does not have security enabled. This is a "private" exception
//!that will be handled by the server.
class incompatible_client_exception
	:public std::runtime_error {
	public:
		incompatible_client_exception()
			:std::runtime_error("Incompatible client!") {

	}
};

//!Exception class that reports the failure of ssl_wrapper::send and
//!client_writer::send. This is a "private" exception that will not be thrown
//!to the outside world.
class send_exception
	:public std::runtime_error {
	public:
		send_exception(const std::string& _msg, bool _secure):
			std::runtime_error(_msg+" ["+(_secure ? "secure" : "not_secure")+"]") {

			}
};

//!Exception class reporting a failure to write to a client. This is the
//!exception exposed to the outside world.
class write_exception
	:public std::runtime_error {
	public:
		write_exception(int _blocksize,
			bool _server_secure,
			const char * _client_status,
			int	_sent,
			int _left,
			const std::string& _errmsg
		):std::runtime_error(std::string("Write error.")
			+" read: "+std::to_string(_blocksize)
			+" sent: "+std::to_string(_sent)
			+" left: "+std::to_string(_left)
			+" server_secure: "+std::to_string(_server_secure)
			+" client_status: "+_client_status
			+" errmsg: "+_errmsg
		) {}
};

//!Internal exception representing a failure to read. This exception will be
//!caught by the internals of the listener_service.
class read_exception
	:public std::runtime_error {
	public:
		read_exception(const std::string& _msg, bool _secure)
			:std::runtime_error(_msg+" ["+(_secure ? "secure" : "not_secure")+"]") {
	}
};

}
#endif
