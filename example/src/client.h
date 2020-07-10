#pragma once

#include <sck/client.h>

namespace app {

class client {

	public:

								client(const std::string&, int, bool);
	void                        send(const std::string& _msg) {client_instance.send(_msg);}
	std::string                 receive() {return client_instance.receive();}
	std::string                 wait_for_answer();
	bool                        is_done() const {return done;}

	private:

	sck::client                 client_instance;
	bool						done=false;
};

}
