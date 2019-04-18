#include "client_writer.h"

#include "exception.h"

#include <sys/types.h>
#include <sys/socket.h>


using namespace sck;

//TODO: Inject log...
client_writer::client_writer(openssl_wrapper * _openssl_wrapper)
	:ssl_wrapper(_openssl_wrapper) {

}


void client_writer::write(const std::string& _msg, const connected_client& _cl, bool _allow_downgrade) {

	int 		sent=0,
				left=_msg.size(),
				client_descriptor=_cl.descriptor;

	while(left) {

		bool use_secure=_cl.is_unverified() 
			? is_secure() 
			: (is_secure() && !_cl.is_secure() && _allow_downgrade
				? false
				: is_secure());

		int blocksize=use_secure
			? ssl_wrapper->send(client_descriptor, _msg.substr(sent, left).c_str(), left)
			: send(client_descriptor, _msg.substr(sent, left).c_str(), left, 0);

		if(0 > blocksize) {

			//TODO: This is ugly: ssl_wrapper->send throws itself... Perhaps we need a plain wrapper too.
			throw write_exception(blocksize, is_secure(), _cl.is_secure(), _allow_downgrade, sent, left, translate_error(errno));
		}
		left-=blocksize;
		sent+=blocksize;
	}
}

std::string client_writer::translate_error(int _err) const {

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
