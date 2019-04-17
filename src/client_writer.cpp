#include "client_writer.h"

#include <stdexcept>

#include <sys/types.h>
#include <sys/socket.h>


using namespace sck;

client_writer::client_writer(openssl_wrapper * _openssl_wrapper)
	:ssl_wrapper(_openssl_wrapper) {

}

void client_writer::write(const std::string& _msg, const connected_client& _cl) {

	int 		sent=0,
				left=_msg.size(),
				client_descriptor=_cl.descriptor;

	while(left) {

		//TODO: Again, we would ask if the client is SSL to choose a method or other.
		//TODO: If the connection is 

		//TODO: We can be sure that is_secure can only be set if the server has
		//SSL, which is good.
		//int blocksize=!_cl.is_secure() 

		int blocksize=/*nullptr==ssl_wrapper*/
			? send(client_descriptor, _msg.substr(sent, left).c_str(), left, 0)
			: ssl_wrapper->send(client_descriptor, _msg.substr(sent, left).c_str(), left);

		if(0 > blocksize) {
			//TODO: Could use a specialised exception to store how much did we send.
			throw std::runtime_error("Could not write to client socket");
		}
		left-=blocksize;
		sent+=blocksize;
	}
}
