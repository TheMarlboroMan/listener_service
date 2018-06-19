#ifndef CLIENT_WRITER_H
#define CLIENT_WRITER_H

#include "connected_client.h"

namespace sck {

class client_writer {

	public:

	void write(const std::string&, const connected_client&);
};

}

#endif
