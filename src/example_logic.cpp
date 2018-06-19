#include "example_logic.h"

using namespace sck;

example_logic::example_logic(server& _s):srv(_s) {

}

void example_logic::handle_new_connection(const connected_client& _c) {

	//TODO: Could create a message wrapper.
	wrt.write("Welcome to the service\n", _c);

}

void example_logic::handle_client_data(const std::string& _msg, const connected_client& _c) {

	//TODO: Could create a message wrapper.
	wrt.write("Ack", _c);

	if(_msg=="FINISH\n") {
		srv.stop();
	}
}

void example_logic::handle_dissconection(const connected_client& _c) {

	//TODO: Could create a message wrapper.
	wrt.write("Bye", _c);
}
