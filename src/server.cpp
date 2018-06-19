#include "server.h"

#include <iostream>
//#include <thread>

#include <cstring> 	//Memset.
#include <arpa/inet.h> 	//inet_ntop.
#include <signal.h> 	//Memset.
#include <sys/un.h>	//Local sockets...

#include "logtools.h"

extern tools::log srvlog;

/*
struct addrinfo {
    int              ai_flags;     // AI_PASSIVE, AI_CANONNAME, etc.
    int              ai_family;    // AF_INET, AF_INET6, AF_UNSPEC
    int              ai_socktype;  // SOCK_STREAM, SOCK_DGRAM
    int              ai_protocol;  // use 0 for "any"
    size_t           ai_addrlen;   // size of ai_addr in bytes
    struct sockaddr *ai_addr;      // struct sockaddr_in or _in6
    char            *ai_canonname; // full canonical hostname

    struct addrinfo *ai_next;      // linked list, next node
};

struct sockaddr {
    unsigned short    sa_family;    // address family, AF_xxx
    char              sa_data[14];  // 14 bytes of protocol address
};

struct sockaddr_in {
    short int          sin_family;  // Address family, AF_INET
    unsigned short int sin_port;    // Port number
    struct in_addr     sin_addr;    // Internet address
    unsigned char      sin_zero[8]; // Same size as struct sockaddr
};
*/

using namespace sck;

server::server(int _p, int _bs, int _b)
	:read_message_buffer_size(_bs), port(_p), backlog(_b) {

	signal(SIGINT, &server::handle_signint);
	read_message_buffer=new char[read_message_buffer_size];
	memset(read_message_buffer, 0, read_message_buffer_size);
}

server::~server() {

	cleanup();
}

void server::cleanup() {

	tools::info()<<"Cleaning up..."<<tools::endl();

	if(read_message_buffer) {
		delete [] read_message_buffer;
		read_message_buffer=nullptr;
	}

	//TODO: This will DUMP!!!.
	for(int i=0; i<=in_sockets.max_descriptor; i++) {
		if(i!=file_descriptor) {
			std::cout<<"DISCONNECT "<<i<<std::endl;
			disconnect_client(i);
		}
	}
}

void server::handle_signint(int _s) {
	tools::info()<<"Caught SIGINT. Exiting..."<<tools::endl();
	exit(1);
}


/*
From man getaddrinfo:

If the AI_PASSIVE flag is specified in hints.ai_flags, and node is NULL, then the returned socket  addresses  will
be suitable for bind(2)ing a socket that will accept(2) connections.  The returned socket address will contain the
"wildcard address" (INADDR_ANY for IPv4 addresses, IN6ADDR_ANY_INIT for IPv6 address).  The  wildcard  address  is
used  by  applications  (typically  servers)  that  intend  to  accept  connections  on any of the hosts's network
addresses.  If node is not NULL, then the AI_PASSIVE flag is ignored.

If the AI_PASSIVE flag is not set in hints.ai_flags, then the returned socket addresses will be suitable  for  use
with  connect(2), sendto(2), or sendmsg(2).  If node is NULL, then the network address will be set to the loopback
interface address (INADDR_LOOPBACK for IPv4 addresses, IN6ADDR_LOOPBACK_INIT for IPv6 address); this  is  used  by
applications that intend to communicate with peers running on the same host.
*/

void server::start() {

	//TODO: This could not be uglier.

	//Fill up the hints...
	addrinfo hints;
	memset(&hints, 0, sizeof hints);
	hints.ai_family=AF_UNSPEC;
	hints.ai_socktype=SOCK_STREAM;
	hints.ai_flags=AI_PASSIVE;

	//Get the address info.
	addrinfo *servinfo=nullptr;
	//We could use "127.0.0.1", it would not give us the wildcard 0.0.0.0.
	int getaddrinfo_res=getaddrinfo(nullptr, std::to_string(port).c_str(), &hints, &servinfo);
	if(getaddrinfo_res != 0) {
		throw std::runtime_error("Failed to get address info :"+std::string(gai_strerror(getaddrinfo_res)));
	}

	//Bind to the first good result.
	addrinfo *p=servinfo;
	while(nullptr!=p) {
		file_descriptor=socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if(-1!=file_descriptor) {

			//Force adress reuse, in case a previous process is still hogging the port.
			int optval=1;
			if(setsockopt(file_descriptor, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int))==-1) {
				throw std::runtime_error("Unable to force address reuse");
			}

			if(bind(file_descriptor, p->ai_addr, p->ai_addrlen) == -1) {
				close(file_descriptor);
				continue;
			}

			address=ip_from_addrinfo(*p);
			break; //Everything is ok... Break the loop!.
		}
		p=p->ai_next;
	}

	freeaddrinfo(servinfo);
	if(!p) {
		throw std::runtime_error("Could not bind socket! Perhaps the port is still in use?");
	}

	if(::listen(file_descriptor, backlog)==-1) {
		throw std::runtime_error("Could not set the server to listen");
	}

	tools::info()<<"Server started on "<<address<<":"<<port<<tools::endl();

	in_sockets.max_descriptor=file_descriptor > in_sockets.max_descriptor ? file_descriptor : in_sockets.max_descriptor;
	FD_ZERO(&in_sockets.set);
	FD_SET(file_descriptor, &in_sockets.set);

	if(file_descriptor==-1) {
		throw std::runtime_error("Cannot run if server is not started");
	}

	loop();
}

void server::stop() {

	tools::info()<<"Stopping server now. Will complete the current listening cycle."<<tools::endl();
	running=false;
}

void server::loop() {

	fd_set copy_in;
	//timeval timeout{0, 100000}; //Struct of 0.1 seconds.
	timeval timeout{1, 0}; //Struct of 1 seconds. Select will exit once anything is ready, regardless of the timeout.
	tools::info()<<"Starting to listen now"<<tools::endl();

	running=true;

	//TODO: Do we have a 100% loop?
	while(running) {
		try {
			copy_in=in_sockets.set;	//Swap... select may change its values!.
			timeout.tv_sec=5;
			timeout.tv_usec=0;	//select MAY write on this values and cause us to hog CPU.

			int select_res=select(in_sockets.max_descriptor+1, &copy_in, nullptr, nullptr, &timeout);
			if(select_res==-1) {
				throw std::runtime_error("Select failed!");
			}

			if(select_res > 0) {
				for(int i=0; i<=in_sockets.max_descriptor; i++) {
					if(FD_ISSET(i, &copy_in)) {
						if(i==file_descriptor) { //New connection on listener file_descriptor.
							handle_new_connection();
						}
						else { //New data on client file descriptor.
							handle_client_data(i);
						}
					}
				}
			}
		}
		catch(std::exception &e) {
			tools::error()<<"Listener thread caused an exception: "<<e.what()<<tools::endl();
		}
	}

	tools::info()<<"Listening stopped"<<tools::endl();
}

void server::handle_new_connection() {

	sockaddr_in client_address;
	socklen_t l=sizeof(client_address);
	int client_descriptor=accept(file_descriptor, (sockaddr *) &client_address, &l);

	if(client_descriptor==-1) {
		throw std::runtime_error("Failed on accept for new connection");
	}

	clients.insert( {client_descriptor, connected_client(client_descriptor, ip_from_sockaddr_in(client_address))} );
	FD_SET(client_descriptor, &in_sockets.set);
	in_sockets.max_descriptor=client_descriptor > in_sockets.max_descriptor ? client_descriptor : in_sockets.max_descriptor;

	if(logic) {
		logic->handle_new_connection(clients.at(client_descriptor));
	}

	tools::info()<<"Client "<<client_descriptor<<" connected from "<<clients.at(client_descriptor).ip<<tools::endl();
}

void server::handle_client_data(int _file_descriptor) {

	try {
		std::string message=read_from_socket(_file_descriptor);

		if(logic) {
			logic->handle_client_data(message, clients.at(_file_descriptor));
		}
	}
	catch(client_disconnected_exception& e) {
		tools::info()<<"Client "<<_file_descriptor<<" disconnected on client side..."<<tools::endl();
		disconnect_client(_file_descriptor);
	}
}

void server::disconnect_client(int _client_key) {

	if(!clients.count(_client_key)) {
		throw std::runtime_error("Attempted to disconnect invalid client");
	}

	if(logic) {
		logic->handle_dissconection(clients.at(_client_key));
	}

	//Well, the key is actually the file descriptor, but that could change.
	int file_descriptor=clients.at(_client_key).descriptor;

	close(file_descriptor);
	FD_CLR(file_descriptor, &in_sockets.set);
	clients.erase(_client_key);

	tools::info()<<"Client "<<file_descriptor<<" disconnected"<<tools::endl();
}

/*
From man recv:

If a message is too long to fit in the supplied buffer, excess bytes may be
discarded depending on the type of socket the message is received from...

We tested it: it is not discarded here, just waiting for us to read_from_socket
again... So well, we should actually try to compose a message somehow!.
*/

std::string server::read_from_socket(int _client_descriptor) {

	memset(read_message_buffer, 0, read_message_buffer_size);
	auto read=recv(_client_descriptor, read_message_buffer, read_message_buffer_size-1, 0);

	if(read==-1) {
		throw std::runtime_error("Could not read from client socket in client_descriptor "+std::to_string(_client_descriptor));
	}
	else if(read==0) {
		throw client_disconnected_exception("Client disconnected!");
	}

	return std::string(read_message_buffer);
}

void server::set_logic(logic_interface& li) {

	logic=&li;
}

//TODO: Suckage: fix the calling point and pass the ai_addr damn it...
std::string sck::ip_from_addrinfo(const addrinfo& p) {
	//gethostname...
	char ip[INET6_ADDRSTRLEN];
	inet_ntop(p.ai_family, &(reinterpret_cast<sockaddr_in*>(p.ai_addr))->sin_addr, ip, sizeof ip);
	return std::string(ip);
}

std::string sck::ip_from_sockaddr_in(const sockaddr_in& p) {
	//getpeername...
	char ip[INET6_ADDRSTRLEN];
	inet_ntop(p.sin_family, &(p.sin_addr), ip, sizeof ip);
	return std::string(ip);
}
