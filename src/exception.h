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

class incompatible_client_exception
	:public std::runtime_error {
	public:
		incompatible_client_exception()
			:std::runtime_error("Incompatible client!") {

	}
};

class write_exception
	:public std::runtime_error {
	public:
		write_exception(int _blocksize, 
			bool _server_secure, 
			bool _client_secure, 
			bool _server_allows_downgrade,
			int	_sent,
			int _left,
			const std::string& _errmsg
		):std::runtime_error(std::string("Write error.")
			+" blocksize: "+std::to_string(_blocksize)
			+" sent: "+std::to_string(_sent)
			+" left: "+std::to_string(_left)
			+" server_secure: "+std::to_string(_server_secure)
			+" client_secure: "+std::to_string(_client_secure)
			+" server_allows_downgrade: "+std::to_string(_server_allows_downgrade)
			+" errmsg: "+_errmsg
		) {}
};

}
#endif
