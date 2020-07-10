#include <sck/client_writer.h>
#include <sck/exception.h>

#include <sys/types.h>
#include <sys/socket.h>


using namespace sck;

client_writer::client_writer(openssl_wrapper * _openssl_wrapper)
	:ssl_wrapper(_openssl_wrapper) {

}

void client_writer::write(const std::string& _msg, const connected_client& _cl) {

	int 		sent=0,
				blocksize=0,
				left=_msg.size(),
				client_descriptor=_cl.descriptor;

	while(left) {

		try {
			//Notice that there is no way that the server is not secure and
			//the client is: these get disconnected.

			blocksize=is_secure() && _cl.is_secure()
				? ssl_wrapper->send(client_descriptor, _msg.substr(sent, left).c_str(), left)
				: send(client_descriptor, _msg.substr(sent, left), left, 0);

			left-=blocksize;
			sent+=blocksize;
		}
		catch(send_exception &e) {
			throw write_exception(blocksize, is_secure(), _cl.get_readable_status(), sent, left, e.what());
		}
	}
}

int client_writer::send(int _client_descriptor, const std::string& _message, int _len, int _flags) {

	int blocksize=::send(_client_descriptor, _message.c_str(), _len, _flags);

	if(0 > blocksize) {
		throw send_exception(translate_error(errno), false);
	}

	return blocksize;
}

std::string client_writer::translate_error(int _err) const {

	//TODO: not really neccesary, chedck perror and family.

	switch(_err) {
		case EACCES: return "EACCES";
		case EAGAIN: return "EAGAIN";
//		case EWOULDBLOCK: return "EWOULDBLOCK"; //Same as EAGAIN.
		case EBADF: return "EBADF";
		case ECONNRESET: return "ECONNRESET";
		case EDESTADDRREQ: return "EDESTADDRREQ";
		case EFAULT: return "EFAULT";
		case EINTR: return "EINTR";
		case EINVAL: return "EINVAL";
		case EISCONN: return "EISCONN";
		case EMSGSIZE: return "EMSGSIZE";
		case ENOBUFS: return "ENOBUFS";
		case ENOMEM: return "ENOMEM";
		case ENOTCONN: return "ENOTCONN";
		case ENOTSOCK: return "ENOTSOCK";
		case EOPNOTSUPP: return "EOPNOTSUPP";
		case EPIPE: return "EPIPE";
	}

	return "unknown";
}
