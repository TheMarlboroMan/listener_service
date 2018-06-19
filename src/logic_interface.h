#ifndef LOGIC_INTERFACE_H
#define LOGIC_INTERFACE_H

#include <string>

namespace sck {

class connected_client;

class logic_interface {

	public:

	virtual void 		handle_new_connection(const connected_client&)=0;
	virtual void 		handle_client_data(const std::string&, const connected_client&)=0;
	virtual void 		handle_dissconection(const connected_client&)=0;
};

}

#endif
