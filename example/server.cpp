#include "src/example_logic.h"

#include <sck/server.h>

#include <tools/arg_manager.h>
#include <lm/file_logger.h>

#include <cstdlib>
#include <signal.h>

#include <iostream>
#include <stdexcept>


//Test client with netcat 127.0.1.1 16666 or echo "HI" | netcat 127.0.0.1 16666
int use(int _v);
void handle_sigint(int _s);

int main(int argc, char ** argv) {

	try {
		signal(SIGINT, handle_sigint);

		tools::arg_manager argman(argc, argv);

		if(argman.size() < 3) {
			return use(1);
		}

		//Get port and init the server...
		int port_index=argman.find_index("-p");
		int sockfile_index=argman.find_index("-f");
		if( (-1==port_index && -1 ==sockfile_index) || (-1!=port_index && -1!=sockfile_index)) {
			return use(2);
		}

		sck::server_config cfg;
		if(-1!=port_index) {
			int port=std::atoi(argman.get_argument(port_index+1).c_str());
			if(0==port) {
				throw std::runtime_error("Invalid port");
			}
			cfg.socktype=sck::server_config::type::sock_inet;
			cfg.port=port;
		}
		else {
			cfg.socktype=sck::server_config::type::sock_unix;
			cfg.unix_sock_path=argman.get_argument(sockfile_index+1);
		}

		if(-1!=argman.find_index("-ssl")) {
			cfg.use_ssl_tls=true;
			cfg.ssl_tls_cert_path="cert.pem";
			cfg.ssl_tls_key_path="key.pem";
			cfg.ssl_set_security_milliseconds=250;
			cfg.ssl_set_security_seconds=5;
		}

		//Manage log.
		std::unique_ptr<lm::logger> logger{nullptr};

		int log_index=argman.find_index("-l");
		if(-1!=log_index) {

			logger.reset(new lm::file_logger(argman.get_argument(log_index+1).c_str()));
		}

		sck::server srv(cfg, logger.get());

		//Manage logic.
		app::example_logic exl(srv, logger.get());
		srv.set_logic(exl);

		//Daemonize if needed...
		if(-1!=argman.find_index("-d")) {
			std::cout<<"Running as daemon"<<std::endl;
			 //do not change working directory, redirect to dev/null.
			if(0!=daemon(1, 0)) {
				throw std::runtime_error("Could not daemonize");
			}
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

int use(int _v) {

	std::cout<<"["<<_v<<"] use: ./server.out -p #port|-f #file [-l #logfile] [-d] [-ssl]\n"
"\t-p Port number\n"
"\t-f Socket filename\n"
"\t-l Log file to use\n"
"\t-d Run as daemon\n"
"\t-ssl Enable SSL"<<std::endl;

	return _v;
}

void handle_sigint(int /*_s*/) {

	exit(1);/*  */
}
