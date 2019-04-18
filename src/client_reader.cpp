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

std::string client_reader::read(connected_client& _client) {

//	_allow_downgrade=true;

	memset(read_message_buffer, 0, read_message_buffer_size);

	//As with the writer class, notice that there is no way that the server
	//is not secure and the client is: these get disconnected.

	is_secure() && _client.is_secure()
		? ssl_wrapper->recv(_client.descriptor, read_message_buffer, read_message_buffer_size-1)
		: recv(_client.descriptor);

	return std::string(read_message_buffer);
}

int client_reader::recv(int _client_descriptor) {

	int read=::recv(_client_descriptor, read_message_buffer, read_message_buffer_size-1, 0);
	if(0==read) {

		throw client_disconnected_exception();
	}

	if(-1==read) {
		throw read_exception(translate_error(errno), false);
	}

	return read;
}

const char * client_reader::translate_error(int _err) const {

	switch(_err) {
		case EAGAIN: return "EAGAIN";
//		case EWOULDBLOCK: return "EWOULDBLOCK"; Same as EAGAIN... at least in this compiler.
		case EBADF: return "EBADF";
		case ECONNREFUSED: return "ECONNREFUSED";
		case EFAULT: return "EFAULT";
		case EINTR: return "EINTR";
		case EINVAL: return "EINVAL";
		case ENOMEM: return "ENOMEM";
		case ENOTCONN: return "ENOTCONN";
		case ENOTSOCK: return "ENOTSOCK";
	}

	return "unknown";
}
