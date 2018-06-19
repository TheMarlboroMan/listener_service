#include <iostream>

#include <thread>

#include "src/server.h"
#include "src/logtools.h"

//Test client with netcat 127.0.1.1 16666 or echo "HI" | netcat 127.0.0.1 16666
tools::log srvlog("logs/srvlog.log");

int main(int argc, char ** argv) {

	try {

		//TODO: Use parameter for port.
		sck::server srv(16666);
		srv.start();
		return 0;
	}
	catch(std::exception &e) {

		tools::error()<<"Aborting due to exception: "<<e.what()<<tools::endl();
		std::cout<<"Something failed: "<<e.what()<<std::endl;
		return 1;
	}
}
