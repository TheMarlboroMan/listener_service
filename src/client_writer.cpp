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

		int blocksize=!_cl.is_secure()
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
