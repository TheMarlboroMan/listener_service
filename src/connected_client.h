#ifndef CONNECTED_CLIENT_H
#define CONNECTED_CLIENT_H

#include <string>

namespace sck {

struct connected_client {
	//!Class constructor.

					connected_client(int _fd, const std::string& _ip, bool _secure)
		:descriptor(_fd), ip(_ip), secure(_secure) {
	}

	bool			is_secure() const {return secure;}

	//!File descriptor id.
	int				descriptor;

	//!Client ip.
	std::string		ip;

	private:

	//!True if the client is using TLS.
	bool			secure;

};

}

#endif
