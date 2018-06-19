#include <iostream>
#include <stdexcept>

#include <cstdlib>

#include "src/server.h"
#include "src/logtools.h"
#include "src/example_logic.h"

#include <arg_manager.h>

//Test client with netcat 127.0.1.1 16666 or echo "HI" | netcat 127.0.0.1 16666
tools::log srvlog("logs/srvlog.log");

int use(int _v) {

	std::cout<<"use: ./a.out -p #port";
	return _v;
}

int main(int argc, char ** argv) {

	try {

		tools::arg_manager argman(argc, argv);

		if(3!=argman.size()) {
			return use(1);
		}
		else if(1!=argman.find_index_value("-p")) {
			return use(1);
		}


		int port=std::atoi(argman.get_argument(2).c_str());
		if(0==port) {
			throw std::runtime_error("Invalid port");
		}

		sck::server srv(port);

		sck::example_logic exl(srv);
		srv.set_logic(exl);
		srv.start();
		return 0;
	}
	catch(std::exception &e) {

		tools::error()<<"Aborting due to exception: "<<e.what()<<tools::endl();
		std::cout<<"Something failed: "<<e.what()<<std::endl;
		return 1;
	}
}
