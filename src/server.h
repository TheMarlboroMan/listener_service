#ifndef SOCKET_SERVER_H
#define SOCKET_SERVER_H

#include <string>
#include <map>
#include <memory>

#include <atomic>
#include <thread>
#include <netinet/in.h>	//This is for domain adresses.
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h> //read, write...

#include <src/log.h>

#include "connected_client.h"
#include "logic_interface.h"
#include "client_writer.h"
#include "client_reader.h"
#include "openssl_wrapper.h"

namespace sck {

//!Utility function to extract an IP from an addrinfo struct.
std::string ip_from_addrinfo(const addrinfo&);
//!Utility function to extract an IP from an sockaddr_in struct.
std::string ip_from_sockaddr_in(const sockaddr_in&);


struct server_config {

	int			port,
				blocksize=1024,
				backlog=5,
				ssl_set_security_seconds=1,
				ssl_set_security_milliseconds=0;
	bool			use_ssl_tls=false;
	std::string		ssl_tls_cert_path,
				ssl_tls_key_path;
};

//!Main server.
class server {

	public:

	//!Builds the server with the given port, message buffer size and backlog.
						server(const server_config&, tools::log * =nullptr);
	//!Class destructor.
						~server();
	//!Starts the server: mainly sets up the socket.
	void				start();
	//!Stops the server by setting the running flag to false.
	void 				stop();
	//!Sets the external business logic.
	void 				set_logic(logic_interface&);
	//!Forces the disconnection of the given client.
	void 				disconnect_client(const connected_client&);
	//!Obtains a client writer, imbuing it with any neccesary data.
	client_writer		create_writer();

	//!Returns true if SSL/TLS is enabled. Always returns false if
	//compiled without SSL.
	bool				is_secure() const;

	private:

	//!Internal main loop.
	void				loop();
	//!Reads data from the file descriptor.
	std::string			read_from_socket(int);
	//!Internal handler for incoming connections.
	void				handle_new_connection();
	//!Internal handler for messages from existing connections.
	void				handle_client_data(connected_client&);
	//!After a connection is accepted, the server will try to ascertain if
	//!the connection is secure or not.
	void 				set_client_security(connected_client&);

	//!Will call "set_secure on the client", offering also a callback possibility for logic.
	void				secure_client(connected_client&, bool);

	std::unique_ptr<openssl_wrapper>	ssl_wrapper;
	client_reader						reader;
	std::map<int, connected_client>		clients;	//!<Maps file descriptors to client data.

	server_config		config;

	int					file_descriptor=-1;
						
	std::string			address;
	logic_interface	*	logic=nullptr;
	tools::log *		log=nullptr;
	std::atomic<int>	security_thread_count;

	struct {
		fd_set 			set;
		int				max_descriptor=0;
	}					in_sockets;

	//TODO: Document...
	struct security_thread_count_guard {

		security_thread_count_guard(std::atomic<int>& _count):
			count{_count} {
			++count;
		}
		~security_thread_count_guard() {--count;}
		std::atomic<int>&			count;
	};

	bool				running=false;
};

}

#endif
