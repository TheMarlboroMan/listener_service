#ifndef CONNECTED_CLIENT_H
#define CONNECTED_CLIENT_H

#include <string>

namespace sck {

struct connected_client {

	int				descriptor;
	std::string		ip;
					connected_client(int _fd, const std::string& _ip)
		:descriptor(_fd), ip(_ip) {
	}
};

}

#endif
