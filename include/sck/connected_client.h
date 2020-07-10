#pragma once

#include <string>

namespace sck {

class connected_client {

	public:
	//!Class constructor.
					connected_client(int _fd, const std::string& _ip)
		:descriptor(_fd), ip(_ip), status(status_types::unverified) {
	}

	void			set_secure() {status=status_types::secure;}
	void			set_not_secure() {status=status_types::not_secure;}

	bool			is_secure() const {return status_types::secure==status;}
	bool			is_not_secure() const {return status_types::not_secure==status;}
	bool			is_verified() const {return status_types::unverified!=status;}
	bool			is_unverified() const {return status_types::unverified==status;}

	const char *	get_readable_status() const {

		switch(status) {
			case status_types::secure: return "secure";
			case status_types::not_secure: return "not_secure";
			case status_types::unverified: return "unverified";
		}

		//Shut up compiler...
		return "";
	}


	//!File descriptor id.
	int			descriptor;

	//!Client ip.
	std::string		ip;

	private:

	//!SSL/TLS status. Cannot be verified until the client speaks to the
	//!server, hence unverified.
	enum class 		status_types{unverified, secure, not_secure}	status;
};

}

