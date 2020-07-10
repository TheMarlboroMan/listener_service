#pragma once

#include <string>
#include "exception.h"

namespace sck {

class connected_client;

//!Defines a public interface for logic objects. Logic objects are those who
//!contain the businness logic of the aplication, independent of all the
//!network operations going on in the server object.
//!An interesting bit: when a class is created that implements this interface
//!it just makes sense to build it with a client_writer, to enable communication
//!through the specific server config.
class logic_interface {

	public:

	//!Must instruct the application on what to do when receiving a new connection.
	//!Bear in mind that this new client's SSL/TLS info is yet to be confirmed.
	virtual void 		handle_new_connection(const connected_client&)=0;

	//!Must instruct the application on what to do when getting new data.
	//!The client's SSL/TLS info is now known.
	virtual void 		handle_client_data(const std::string&, const connected_client&)=0;

	//!Must instruct the application on what to do when a client disconnects.
	//!The client's SSL/TLS info might or might not be known.
	virtual void 		handle_dissconection(const connected_client&)=0;

	//!Must instruct the application on what to do when a client is deemed
	//!secure or not secure. At this point, the client's SSL/TLS status is
	//!definitely known.
	virtual void 		handle_client_security(const connected_client&, bool)=0;

	//!Must instruct the application on what to do when the underlying server
	//!exits its main loop.
	virtual void 		handle_server_shutdown()=0;

	//!Must instruct the application on what to do when a library exception is
	//!caught. Currently it is always supported on the read_exception and
	//!client_disconnected_exception classes.
	virtual void		handle_exception(exception&, const connected_client&)=0;
};

}

