#ifndef EXAMPLE_LOGIC_H
#define EXAMPLE_LOGIC_H

#include <src/log.h>

#include "../../src/server.h"
#include "../../src/logic_interface.h"
#include "../../src/connected_client.h"
#include "../../src/client_writer.h"

namespace app {

class example_logic:
						public sck::logic_interface {

	public:

						example_logic(sck::server&, tools::log*);

	virtual void 		handle_new_connection(const sck::connected_client&);
	virtual void 		handle_client_data(const std::string&, const sck::connected_client&);
	virtual void 		handle_dissconection(const sck::connected_client&);

	private:

	void				write(const std::string&, const sck::connected_client&);

	sck::server&		srv;
	sck::client_writer	wrt;
	tools::log *		log;


};
}
#endif
