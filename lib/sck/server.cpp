#include <sck/server.h>
#include <sck/exception.h>

#include <lm/log.h>

#include <cstring> 	//Memset.
#include <arpa/inet.h> 	//inet_ntop.
#include <signal.h> 	//Memset.
#include <sys/un.h>	//Local sockets...

#include <iostream>

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

server::server(const server_config& _sc, lm::logger * _log)
	:
	//TODO: Why don't just keep the sc instance and refer to it?????
	ssl_wrapper(_sc.use_ssl_tls
		? new openssl_wrapper(_sc.ssl_tls_cert_path, _sc.ssl_tls_key_path, _log)
		: nullptr),
	reader{_sc.blocksize, ssl_wrapper.get()},
	config(_sc),
	log(_log),
	security_thread_count{0} {

}

server::~server() {

	if(log) {
		lm::log(*log).info()<<"Cleaning up clients..."<<std::endl;
	}

	for(int i=0; i<=in_sockets.max_descriptor; i++) {
		if(clients.count(i)) {
			disconnect_client(clients.at(i));
		}
	}

	ssl_wrapper.reset(nullptr);

	if(server_config::type::sock_unix==config.socktype) {

		unlink(config.unix_sock_path.c_str());
	}

	if(log) {
		lm::log(*log).info()<<"Cleanup completed..."<<std::endl;
	}
}

bool server::is_secure() const {

	return nullptr!=ssl_wrapper;
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

	switch(config.socktype) {

		case server_config::type::sock_unix:
			setup_unix();
		break;
		case server_config::type::sock_inet:
			setup_inet();
		break;
	}

	if(::listen(file_descriptor, config.backlog)==-1) {

		throw std::runtime_error("Could not set the server to listen");
	}

	in_sockets.max_descriptor=file_descriptor > in_sockets.max_descriptor ? file_descriptor : in_sockets.max_descriptor;
	FD_ZERO(&in_sockets.set);
	FD_SET(file_descriptor, &in_sockets.set);

	if(file_descriptor==-1) {
		throw std::runtime_error("Cannot run if server is not started");
	}

	loop();
}

void server::setup_inet() {

	//Fill up the hints...
	addrinfo hints;
	memset(&hints, 0, sizeof hints);
	hints.ai_family=AF_UNSPEC;
	hints.ai_socktype=SOCK_STREAM;
	hints.ai_flags=AI_PASSIVE;

	//Get the address info.
	addrinfo *servinfo=nullptr;
	//TODO: Specify the IP in which we are listening...
	//We could use "127.0.0.1", it would not give us the wildcard 0.0.0.0.
	int getaddrinfo_res=getaddrinfo(nullptr, std::to_string(config.port).c_str(), &hints, &servinfo);
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

			if(0==bind(file_descriptor, p->ai_addr, p->ai_addrlen)) {
				address=ip_from_addrinfo(*p);
				break; //Everything is ok... Break the loop!.
			}

			close(file_descriptor);
		}
		p=p->ai_next;
	}

	freeaddrinfo(servinfo);
	if(!p) {
		throw std::runtime_error("Could not bind socket! Perhaps the port is still in use?");
	}

	if(log) {
		lm::log(*log).info()<<"Inet server started on "<<address<<":"<<config.port<<" with FD "<<file_descriptor<<std::endl;
	}

}

void server::setup_unix() {

	file_descriptor=socket(AF_UNIX, SOCK_STREAM, 0);
	if(-1==file_descriptor) {

		throw std::runtime_error("Could not generate file descriptor for local socket");
	}

	//Directly from the man pages.
	struct sockaddr_un my_addr;
	memset(&my_addr, 0, sizeof(struct sockaddr_un));
	my_addr.sun_family=AF_UNIX;
	strncpy(my_addr.sun_path, config.unix_sock_path.c_str(), sizeof(my_addr.sun_path) - 1);
	if(bind(file_descriptor, (struct sockaddr *) &my_addr, sizeof(struct sockaddr_un)) == -1) {

		throw std::runtime_error("Could not bind local socket");
	}

	if(log) {
		lm::log(*log).info()<<"Unix server started on "<<config.unix_sock_path<<" with FD "<<file_descriptor<<std::endl;
	}
}

void server::stop() {

	running=false;

	if(security_thread_count) {
		if(log) {
			lm::log(*log).info()<<"Dangling client security threads detected... waiting. "<<security_thread_count<<" threads remain..."<<std::endl;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(150));
	}

	if(log) {
		lm::log(*log).info()<<"Stopping server now. Will complete the current listening cycle."<<std::endl;
	}
}

void server::loop() {

	fd_set copy_in;
	//timeval timeout{0, 100000}; //Struct of 0.1 seconds.
	timeval timeout{1, 0}; //Struct of 1 seconds. Select will exit once anything is ready, regardless of the timeout.

	if(log) {
		lm::log(*log).info()<<"Starting to listen now"<<std::endl;
	}

	running=true;

	while(running) {

		try {
			copy_in=in_sockets.set;	//Swap... select may change its values!.
			timeout.tv_sec=5;
			timeout.tv_usec=0;	//select MAY write on these values and cause us to hog CPU.

			int select_res=select(in_sockets.max_descriptor+1, &copy_in, nullptr, nullptr, &timeout);
			if(select_res==-1) {
				throw std::runtime_error("Select failed!");
			}

			if(select_res > 0) {
				//TODO: This happens to be reading in stdin and out too :D.
				for(int i=0; i<=in_sockets.max_descriptor; i++) {
					if(FD_ISSET(i, &copy_in)) {
						if(i==file_descriptor) { //New connection on listener file_descriptor.
							handle_new_connection();
						}
						else { //New data on client file descriptor.
							handle_client_data(clients.at(i));
						}
					}
				}
			}
		}
		catch(std::exception &e) {

			if(log) {
				lm::log(*log).info()<<"Listener thread caused an exception: "<<e.what()<<std::endl;
			}
		}
	}

	if(log) {
		lm::log(*log).info()<<"Listening stopped"<<std::endl;
	}

	if(nullptr!=logic) {
		logic->handle_server_shutdown();
	}
}

void server::handle_new_connection() {

	sockaddr_in client_address;
	socklen_t l=sizeof(client_address);

	int client_descriptor=accept(file_descriptor, (sockaddr *) &client_address, &l);
	if(client_descriptor==-1) {
		throw std::runtime_error("Failed on accept for new connection");
	}

	clients.insert(
		{
			client_descriptor,
			connected_client(
				client_descriptor,
				server_config::type::sock_unix==config.socktype
					? "LOCAL"
					: ip_from_sockaddr_in(client_address)
			)
		}
	);
	FD_SET(client_descriptor, &in_sockets.set);
	in_sockets.max_descriptor=client_descriptor > in_sockets.max_descriptor ? client_descriptor : in_sockets.max_descriptor;

	auto& client=clients.at(client_descriptor);

	std::thread client_security_thread(&sck::server::set_client_security, this, std::ref(client));
	client_security_thread.detach();

	if(log) {
		lm::log(*log).info()<<"Client "<<client.descriptor<<" from "<<client.ip<<" status: "<<client.get_readable_status()<<std::endl;
	}

	if(logic) {
		logic->handle_new_connection(client);
	}
}

void server::set_client_security(connected_client& _client) {

	if(log) {
		lm::log(*log).info()<<"Starting thread to determine client "
			<<_client.descriptor<<":"<<_client.ip
			<<" security level, max timeout of "
			<<config.ssl_set_security_seconds<<"sec and "
			<<config.ssl_set_security_milliseconds<<"ms"<<std::endl;
	}

	security_thread_count_guard guard(security_thread_count);

	//A timeout is be used to try and see if the client says something.
	//In the case of SSL/TLS connections the client will speak right away to
	//negotiate the handshake and everything will work out, but not secure
	//clients will stay silent, hence this timeout.

	struct timeval tv;
	tv.tv_sec=config.ssl_set_security_seconds;
	tv.tv_usec=config.ssl_set_security_milliseconds;
	setsockopt(_client.descriptor, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

	char head=0;
	auto recvres=recv(_client.descriptor, &head, 1, MSG_PEEK);

	//Of course, let us reset the timeout.
	tv.tv_sec=0;
	setsockopt(_client.descriptor, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

	//The client did not speak, we just know it is not secure.
	if(-1==recvres) {

		if(log) {
			lm::log(*log).info()<<"Client "
			<<_client.descriptor<<":"<<_client.ip
			<<" is deemed not to use TLS by timeout"<<std::endl;
		}

		secure_client(_client, false);
		return;
	}

	try {

		//22 means start of SSL/TLS handshake.
		if(22==head) {

			//Secure clients connecting to non-secure servers are rejected.
			if(!is_secure()) {
				throw incompatible_client_exception();
			}

			if(log) {
				lm::log(*log).info()<<"Client "
				<<_client.descriptor<<":"<<_client.ip
				<<" uses TLS"<<std::endl;
			}

			secure_client(_client, true);
			ssl_wrapper->accept(_client.descriptor);
		}
		else {
			//A client with no secure capabilities spoke before the timeout...
			if(log) {
				lm::log(*log).info()<<"Client "
				<<_client.descriptor<<":"<<_client.ip
				<<" is deemed not to use TLS by invalid handshake sequence"<<std::endl;
			}

			secure_client(_client, false);
		}
	}
	catch(incompatible_client_exception& e) {

		if(log) {
			lm::log(*log).info()<<"Client "<<_client.descriptor<<" from "<<_client.ip<<" rejected, uses SSL/TLS when server cannot and will be disconnected"<<std::endl;
		}

		disconnect_client(_client);
	}
	catch(openssl_exception &e) {

		if(log) {
			lm::log(*log).info()<<"Client "<<_client.descriptor<<" from "<<_client.ip<<" caused SSL/TLS exception and will be disconnected: "<<e.what()<<std::endl;
		}

		disconnect_client(_client);
	}
}

void server::secure_client(connected_client& _client, bool _secure) {

	if(_secure) {
		_client.set_secure();
	}
	else {
		_client.set_not_secure();
	}

	if(logic) {
		logic->handle_client_security(_client, _secure);
	}
}


void server::handle_client_data(connected_client& _client) {

	//Clients that haven't been validated yet should have their messages
	//ignored.

	try {

		std::string message=reader.read(_client);

		if(logic) {
			logic->handle_client_data(message, _client);
		}
	}
	catch(read_exception& e) {

		if(log) {
			lm::log(*log).info()<<"Client "<<_client.descriptor<<" read failure: "<<e.what()<<std::endl;
		}

		if(logic) {
			logic->handle_exception(e, _client);
		}
	}
	catch(client_disconnected_exception& e) {

		if(log) {
			lm::log(*log).info()<<"Client "<<_client.descriptor<<" disconnected on client side..."<<std::endl;
		}

		if(logic) {
			logic->handle_exception(e, _client);
		}
	}
}

void server::disconnect_client(const sck::connected_client& _cl) {

	int client_key=_cl.descriptor;

	if(!clients.count(client_key)) {

		throw std::runtime_error("Attempted to disconnect invalid client");
	}

	if(logic) {
		logic->handle_dissconection(clients.at(client_key));
	}

	//Well, the key is actually the file descriptor, but that could change.
	int client_fd=clients.at(client_key).descriptor;

	close(client_fd);
	FD_CLR(client_fd, &in_sockets.set);
	clients.erase(client_key);

	if(log) {
		lm::log(*log).info() << "Client " << client_fd << " disconnected" << std::endl;
	}
}

void server::set_logic(logic_interface& _li) {

	logic=&_li;
}

client_writer server::create_writer() {

	return client_writer(ssl_wrapper.get());
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
