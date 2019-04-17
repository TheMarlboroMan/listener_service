#include "example_logic.h"

using namespace sck;

example_logic::example_logic(server& _s)
	:srv(_s), wrt{srv.create_writer()} {

}

void example_logic::handle_new_connection(const connected_client& _c) {

	//TODO: Could create a message wrapper.
	if(wrt.has_ssl()) {
		wrt.write("Welcome to the service: you are using a secure connection\n", _c);
	}
	else {
		wrt.write("Welcome to the service\n", _c);
	}
}

void example_logic::handle_client_data(const std::string& _msg, const connected_client& _c) {

	//TODO: Could create a message wrapper.
	auto m=_msg;
	m.pop_back();
	if(!_msg.size()) {
		return;
	}

	wrt.write("[Ack - "+m+"]\n", _c);

	if(m=="help") {
		wrt.write("Use help for help, stop to shut down the server, exit to disconnect, hi to greet.\n", _c);
	}
	else if(m=="hi") {
		wrt.write("Hi there :)\n", _c);
	}
	else if(m=="stop") {
		srv.stop();
	}
	else if(m=="exit") {
		wrt.write("Go suck a lemon\n", _c);
		srv.disconnect_client(_c);
	}
}

void example_logic::handle_dissconection(const connected_client& _c) {

	//TODO: Could create a message wrapper.
	wrt.write("Bye\n", _c);
}