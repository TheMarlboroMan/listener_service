#ifndef CLIENT_WRITER_H
#define CLIENT_WRITER_H

#include "connected_client.h"

namespace sck {

//!Simple class to write data to a client.
class client_writer {

	public:

	//!Writes the given string to a client.
	void write(const std::string&, const connected_client&);
};

}

#endif
