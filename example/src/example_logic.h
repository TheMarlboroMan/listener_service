#pragma once

#include <sck/server.h>
#include <sck/logic_interface.h>
#include <sck/connected_client.h>
#include <sck/client_writer.h>

#include <lm/logger.h>

namespace app {

class example_logic:
						public sck::logic_interface {

	public:

						example_logic(sck::server&, lm::logger*);

	virtual void 		handle_new_connection(const sck::connected_client&);
	virtual void 		handle_client_data(const std::string&, const sck::connected_client&);
	virtual void 		handle_dissconection(const sck::connected_client&);
	virtual void 		handle_client_security(const sck::connected_client&, bool);
	virtual void 		handle_server_shutdown();
	virtual void		handle_exception(sck::exception&, const sck::connected_client&);

	private:

	void				write(const std::string&, const sck::connected_client&);

	sck::server&		srv;
	sck::client_writer	wrt;
	lm::logger *		log;
};
}
