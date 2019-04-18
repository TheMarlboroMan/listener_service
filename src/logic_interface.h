#ifndef LOGIC_INTERFACE_H
#define LOGIC_INTERFACE_H

#include <string>

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
	virtual void 		handle_dissconection(const connected_client&)=0;
};

}

#endif
