#ifndef EXAMPLE_LOGIC_H
#define EXAMPLE_LOGIC_H

#include "../../src/server.h"
#include "../../src/logic_interface.h"
#include "../../src/connected_client.h"
#include "../../src/client_writer.h"

namespace sck {

class example_logic:
						public logic_interface {

	public:

						example_logic(sck::server&);

	virtual void 		handle_new_connection(const connected_client&);
	virtual void 		handle_client_data(const std::string&, const connected_client&);
	virtual void 		handle_dissconection(const connected_client&);

	private:

	//!Generates a valid message, in this case, adding a newline.
	std::string		msg(const std::string&);

	sck::server&		srv;
	sck::client_writer	wrt;

};
}
#endif
