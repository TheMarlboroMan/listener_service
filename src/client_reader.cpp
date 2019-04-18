#include "client_reader.h"

#include <stdexcept>

#include <sys/types.h>
#include <sys/socket.h>

#include "exception.h"

using namespace sck;

client_reader::client_reader(int _bufsize, openssl_wrapper * _openssl_wrapper)
	:read_message_buffer_size(_bufsize), 
	ssl_wrapper(_openssl_wrapper) {

	read_message_buffer=new char[read_message_buffer_size];
	memset(read_message_buffer, 0, read_message_buffer_size);
}

client_reader::~client_reader() {

	if(read_message_buffer) {
		delete [] read_message_buffer;
		read_message_buffer=nullptr;
	}
}

/*
From man recv:

If a message is too long to fit in the supplied buffer, excess bytes may be
discarded depending on the type of socket the message is received from...

We tested it: it is not discarded here, just waiting for us to read_from_socket
again... So well, we should actually try to compose a message somehow!.
*/

std::string client_reader::read(connected_client& _client, bool _allow_downgrade) {

	if(_client.is_unverified()) {
		set_client_security(_client);
	}

	memset(read_message_buffer, 0, read_message_buffer_size);

	bool use_secure=is_secure() && !_client.is_secure() && _allow_downgrade
		? false
		: is_secure();

	auto read=use_secure
		? ssl_wrapper->recv(_client.descriptor, read_message_buffer, read_message_buffer_size-1)
		: recv(_client.descriptor, read_message_buffer, read_message_buffer_size-1, 0);

	if(read < 0) {
		//TODO: Fuck this, throw a read exception...
		throw std::runtime_error("Could not read from client socket in client_descriptor "+std::to_string(_client.descriptor));
	}
	else if(read==0) {
		throw client_disconnected_exception();
	}

	return std::string(read_message_buffer);
}

void client_reader::set_client_security(connected_client& _client) {

	char head=0;
	recv(_client.descriptor, &head, 1, MSG_PEEK);

	//22 means start of SSL/TLS handshake.
	if(22==head) {
		_client.set_secure();
	}
	else {
		_client.set_not_secure();
	}

	if(_client.is_secure()) {

		//Secure clients connecting to non-secure servers are rejected.
		if(!is_secure()) {

			throw incompatible_client_exception();
		}

		ssl_wrapper->accept(_client.descriptor);
	}
}
