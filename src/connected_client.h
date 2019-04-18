#ifndef CONNECTED_CLIENT_H
#define CONNECTED_CLIENT_H

#include <string>

namespace sck {

struct connected_client {
	//!Class constructor.

					connected_client(int _fd, const std::string& _ip)
		:descriptor(_fd), ip(_ip), status(status_types::unverified) {
	}

	void			set_secure() {status=status_types::secure;}
	void			set_not_secure() {status=status_types::not_secure;}
	bool			is_secure() const {return status_types::secure==status;}
	bool			is_not_secure() const {return status_types::not_secure==status;}
	bool			is_unverified() const {return status_types::unverified==status;}

	//!File descriptor id.
	int				descriptor;

	//!Client ip.
	std::string		ip;

	private:

	//!SSL/TLS status. Cannot be verified until the client speaks to the
	//!server, hence unverified.
	enum class status_types{unverified, secure, not_secure}	status;

};

}

#endif
