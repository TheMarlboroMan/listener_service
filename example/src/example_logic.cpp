#include "example_logic.h"

#include <src/log_tools.h>
#include "../../src/exception.h"

using namespace sck;

example_logic::example_logic(server& _s, tools::log * _log)
	:srv(_s), wrt{srv.create_writer()}, log(_log) {

}

void example_logic::handle_new_connection(const connected_client& _c) {

	write("Welcome to the service", _c);
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

	write(ack, _c);

	if(m=="help") {
		write("Use help for help, stop to shut down the server, exit to disconnect, hi to greet, about to get your info.", _c);
	}
	else if(m=="about") {

		std::string about=_c.ip+" "+(_c.is_secure() ? "secure" : "not secure");
		write(about, _c);
	}
	else if(m=="hi") {
		write("Hi there :)", _c);
	}
	else if(m=="stop") {
		srv.stop();
	}
	else if(m=="exit") {
		write("Go suck a lemon", _c);
		srv.disconnect_client(_c);
	}
}

void example_logic::handle_dissconection(const connected_client& _c) {

	if(_c.is_verified()) {
		write("Bye", _c);
	}
}

void example_logic::write(const std::string& _msg, const connected_client& _c) {

	try {
		wrt.write(_msg+"\n", _c);
	}
	catch(openssl_exception& e) {

		if(log) {
			tools::info(*log)<<"Client "<<_c.descriptor<<" SSL failure: "<<e.what()<<tools::endl();
		}
	}
	catch(write_exception& e) {

		if(log) {
			tools::info(*log)<<"Client "<<_c.descriptor<<" error: "<<e.what()<<tools::endl();
		}
	}
}
