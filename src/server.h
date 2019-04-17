#ifndef SOCKET_SERVER_H
#define SOCKET_SERVER_H

#include <stdexcept>
#include <string>
#include <map>

#include <netinet/in.h>	//This is for domain adresses.
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h> //read, write...

#include <src/log.h>

#include "connected_client.h"
#include "logic_interface.h"
#include "client_writer.h"

#ifdef WITH_SSL
#include <memory>
#include "openssl_wrapper.h"
#endif


namespace sck {

//!Utility function to extract an IP from an addrinfo struct.
std::string ip_from_addrinfo(const addrinfo&);
//!Utility function to extract an IP from an sockaddr_in struct.
std::string ip_from_sockaddr_in(const sockaddr_in&);

//!Internal exception thrown when a read comes in and fails.
class client_disconnected_exception:public std::runtime_error {
	public:
		client_disconnected_exception(const std::string& _e):std::runtime_error(_e) {}
};

//!Main server.
class server {

	public:

	//!Builds the server with the given port, message buffer size and backlog.
						server(int, int=1024, int=5);
	//!Class destructor.
						~server();
	//!Starts the server: mainly sets up the socket.
	void				start();
	//!Stops the server by setting the running flag to false.
	void 				stop();
	//!Sets the external business logic.
	void 				set_logic(logic_interface&);
	//!Sets the external logger.
	void 				set_log(tools::log&);
	//!Forces the disconnection of the given client.
	void 				disconnect_client(const sck::connected_client&);
	//!Obtains a client writer, imbuing it with any neccesary data.
	client_writer		create_writer();

#ifdef WITH_SSL
	//!Enables SSL connection. Uses the given certificate and key paths. May throw if
	//!cannot get it to work. Will throw if called twice.
	void				enable_ssl(const std::string&, const std::string&);
	//!Returns true if SSL is enabled.
	bool				has_ssl() const {return nullptr!=ssl_wrapper;}
#endif

	private:

	const int 			read_message_buffer_size;

	//!Internal main loop.
	void				loop();
	//!Reads data from the file descriptor.
	std::string			read_from_socket(int);
	//!Internal handler for incoming connections.
	void				handle_new_connection();
	//!Internal handler for messages from existing connections.
	void				handle_client_data(int);

	std::map<int, connected_client>	clients;	//!<Maps file descriptors to client data.
#ifdef WITH_SSL
	std::unique_ptr<openssl_wrapper>	ssl_wrapper;
#endif
	int					file_descriptor=-1,
						port,
						backlog;
	std::string			address;
	char * 				read_message_buffer=nullptr;
	logic_interface	*	logic=nullptr;
	tools::log *		log=nullptr;

	struct {
		fd_set 			set;
		int				max_descriptor=0;
	}					in_sockets;

	bool				running=false;
};

}

#endif
