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



	std::cout<<"["<<_v<<"] use: ./server.out -p #port [-l #logfile] [-d] [-ssl]\n"
"\t-p Port number\n"
"\t-d Run as daemon\n"
"\t-ssl Enable SSL"<<std::endl;

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

		sck::server_config cfg;

		int port=std::atoi(argman.get_argument(port_index+1).c_str());
		if(0==port) {
			throw std::runtime_error("Invalid port");
		}

		cfg.port=port;
		if(-1!=argman.find_index("-ssl")) {
			cfg.use_ssl_tls=true;
			cfg.ssl_tls_cert_path="cert.pem";
			cfg.ssl_tls_key_path="key.pem";
		}

		//Manage log.
		tools::log srvlog;
		int log_index=argman.find_index("-l");
		if(-1!=log_index) {
			srvlog.init(argman.get_argument(log_index+1).c_str());
			srvlog.activate();
		}

		sck::server srv(cfg, -1!=log_index ? &srvlog : nullptr);

		//Manage logic.
		app::example_logic exl(srv, -1!=log_index ? &srvlog : nullptr);
		srv.set_logic(exl);

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

