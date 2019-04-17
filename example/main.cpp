#include <iostream>
#include <stdexcept>

#include <cstdlib>
#include <signal.h>

#include "../src/server.h"
#include "src/example_logic.h"

#include <class/arg_manager.h>
#include <src/log_tools.h>

//Test client with netcat 127.0.1.1 16666 or echo "HI" | netcat 127.0.0.1 16666
int use(int _v) {

#ifdef WITH_SSL
	std::cout<<"["<<_v<<"] use: ./a.out -p #port [-l #logfile] [-d] [-ssl]\n\t-p Port number\n\t-d Run as daemon -ssl Enable SSL"<<std::endl;
#else
	std::cout<<"["<<_v<<"] use: ./a.out -p #port [-l #logfile] [-d]\n\t-p Port number\n\t-d Run as daemon"<<std::endl;
#endif
	return _v;
}

void handle_sigint(int _s) {

	exit(1);/*  */
}

int main(int argc, char ** argv) {

	try {
		signal(SIGINT, handle_sigint);

		tools::arg_manager argman(argc, argv);

		if(argman.size() < 3) {
			return use(1);
		}

		//Get port and init the server...
		int port_index=argman.find_index("-p");
		if(-1==port_index) {
			return use(2);
		}

		int port=std::atoi(argman.get_argument(port_index+1).c_str());
		if(0==port) {
			throw std::runtime_error("Invalid port");
		}

		sck::server srv(port);

		//Manage logic.
		sck::example_logic exl(srv);
		srv.set_logic(exl);

		//Manage log.
		tools::log srvlog;
		int log_index=argman.find_index("-l");
		if(-1!=log_index) {
			srvlog.init(argman.get_argument(log_index+1).c_str());
			srvlog.activate();
			srv.set_log(srvlog);
		}

		#ifdef WITH_SSL
		if(-1!=argman.find_index("-ssl")) {
			srv.enable_ssl("cert.pem", "key.pem");		
		}
		#endif



		//Daemonize if needed...
		if(-1!=argman.find_index("-d")) {
			std::cout<<"Running as daemon"<<std::endl;
			daemon(1, 0); //do not change working directory, redirect to dev/null.
		}

		srv.start();
		return 0;
	}
	catch(std::exception &e) {

		//TODO...
		//tools::error()<<"Aborting due to exception: "<<e.what()<<tools::endl();
		std::cout<<"Something failed: "<<e.what()<<std::endl;
		return -1;
	}
}

