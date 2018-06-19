#ifndef EXAMPLE_LOGIC_H
#define EXAMPLE_LOGIC_H

#include "server.h"
#include "logic_interface.h"
#include "connected_client.h"
#include "client_writer.h"

namespace sck {

class example_logic:
						public logic_interface {

	public:

						example_logic(sck::server&);

	virtual void 		handle_new_connection(const connected_client&);
	virtual void 		handle_client_data(const std::string&, const connected_client&);
	virtual void 		handle_dissconection(const connected_client&);

	private:

	sck::server&		srv;
	sck::client_writer	wrt;

};
}
#endif
