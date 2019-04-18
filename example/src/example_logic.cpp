#include "example_logic.h"

using namespace sck;

example_logic::example_logic(server& _s)
	:srv(_s), wrt{srv.create_writer()} {

}

void example_logic::handle_new_connection(const connected_client& _c) {

	wrt.write(msg("Welcome to the service"), _c);
}

void example_logic::handle_client_data(const std::string& _msg, const connected_client& _c) {

	auto m=_msg;
	m.pop_back();
	if(!_msg.size()) {
		return;
	}

	std::string ack=_c.is_secure() 
		? "+ [Ack - "+m+"]"
		: "- [Ack - "+m+"]";

	wrt.write(msg(ack), _c);

	if(m=="help") {
		wrt.write(msg("Use help for help, stop to shut down the server, exit to disconnect, hi to greet, about to get your info."), _c);
	}
	else if(m=="about") {

		std::string about=_c.ip+" "+(_c.is_secure() ? "secure" : "not secure");
		wrt.write(msg(about), _c);
	}
	else if(m=="hi") {
		wrt.write(msg("Hi there :)"), _c);
	}
	else if(m=="stop") {
		srv.stop();
	}
	else if(m=="exit") {
		wrt.write(msg("Go suck a lemon"), _c);
		srv.disconnect_client(_c);
	}
}

void example_logic::handle_dissconection(const connected_client& _c) {

	if(_c.is_verified()) {
		wrt.write(msg("Bye"), _c);
	}
}

std::string example_logic::msg(const std::string& _s) {

	return _s+"\n";
}
