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

#include "connected_client.h"

namespace sck {

std::string ip_from_addrinfo(const addrinfo&);
std::string ip_from_sockaddr_in(const sockaddr_in&);

class client_disconnected_exception:public std::runtime_error {
	public:
		client_disconnected_exception(const std::string& _e):std::runtime_error(_e) {}
};

class server {

	public:

					server(int, int=1024, int=5);
					~server();
	void			start();

	private:

	const int 		read_message_buffer_size;

	static void		handle_signint(int);
	void			loop();
	std::string		read_from_socket(int);
	void			write_to_client(int, const std::string&);
	void			disconnect_client(int);
	void			handle_new_connection();
	void			handle_client_data(int);
	void 			cleanup();

	std::map<int, connected_client>	clients;	//Maps file descriptors to client data.
	int				file_descriptor=-1,
					port,
					backlog;
	std::string		address;
	char * 			read_message_buffer=nullptr;

	struct {
		fd_set 		set;
		int			max_descriptor=0;
	}				in_sockets;
};

}

#endif
