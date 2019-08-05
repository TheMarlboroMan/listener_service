#include "example_logic.h"

#include <src/log_tools.h>
#include "../../src/exception.h"

#include <stdio.h>

using namespace app;

example_logic::example_logic(sck::server& _s, tools::log * _log)
	:srv(_s), wrt{srv.create_writer()}, log(_log) {

}

void example_logic::handle_new_connection(const sck::connected_client& _c) {

	//In this example, we show different messages. However, it would be easy
	//to, for example, discard insecure clients: just use the server reference
	//and call "disconnect_client".

	if(!_c.is_verified()) {
		write("Welcome to the service, your connection is yet to be verified...", _c);
	}
	else {
		_c.is_secure()
			? write("Welcome to the service, you are using a secure connection", _c)
			: write("Welcome to the service, you are using an insecure connection", _c);
	}
}

void example_logic::handle_client_data(const std::string& _msg, const sck::connected_client& _c) {

	try {
		if(log) {
			tools::debug(*log)<<"got '"<<_msg<<"'"<<tools::endl();
		}

		if(_c.is_unverified()) {
			
			write("Please, don't say anything until verified. Disconnecting now.", _c);
			srv.disconnect_client(_c);
			return;
		}

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
		else {
			write("Unkown command. Try help", _c);
		}
	}
	catch(std::exception& e) {
		if(log) {
			tools::info(*log)<<"Something failed: "<<e.what()<<tools::endl();
		}
	}
}

void example_logic::handle_dissconection(const sck::connected_client& _c) {

	//!Is there anybody there to catch this????
	//write("Bye", _c);
}

void example_logic::handle_client_security(const sck::connected_client& _c, bool _secure) {

	_secure
		? write("Your connection has been deemed secure, you can talk now", _c)
		: write("Your connection has been deemed non-secure, you can talk now", _c);
}

void example_logic::handle_exception(sck::exception& _e, const sck::connected_client& _client) {

	if(log) {
		tools::info(*log)<<"Server exception caught and rethrown: "<<_e.what()<<tools::endl();
	}

	srv.disconnect_client(_client);
}


void example_logic::write(const std::string& _msg, const sck::connected_client& _c) {

	try {
		wrt.write(_msg+"\n", _c);
	}
	catch(sck::write_exception& e) {

		if(log) {
			tools::info(*log)<<"Client "<<_c.descriptor<<" error: "<<e.what()<<tools::endl();
		}
	}
}


void example_logic::handle_server_shutdown() {

	if(log) {
		tools::info(*log)<<"This is the example logic handling the server shutdown"<<tools::endl();
	}
}