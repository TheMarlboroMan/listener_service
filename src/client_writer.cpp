#include "client_writer.h"

#include <stdexcept>

#include <sys/types.h>
#include <sys/socket.h>


using namespace sck;

void client_writer::write(const std::string& _msg, const connected_client& _cl) {

	int 		sent=0,
				left=_msg.size(),
				client_descriptor=_cl.descriptor;

	while(left) {

		//TODO: SSL... again.
		int blocksize=send(client_descriptor, _msg.substr(sent, left).c_str(), left, 0);
		if(blocksize==-1) {
			//TODO: Could use a specialised exception to store how much did we send.
			throw std::runtime_error("Could not write to client socket");
		}
		left-=blocksize;
		sent+=blocksize;
	}
}
