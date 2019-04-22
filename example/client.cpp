#include <iostream>
#include <cstdio>
#include <limits>

#include <signal.h>

#include <class/arg_manager.h>

#include "src/client.h"


//Test client with netcat 127.0.1.1 16666 or echo "HI" | netcat 127.0.0.1 16666
int use(int _v) {

	std::cout<<"["<<_v<<"] use: ./client.out -h #host -p #port [-ssl]\n"
"\t-h Host name\n"
"\t-p Port number\n"
"\t-ssl Enable SSL"<<std::endl;

	return _v;
}

void handle_sigint(int _s) {

	exit(1);
}


int main(int argc, char ** argv) {

	try {
		signal(SIGINT, handle_sigint);

		tools::arg_manager argman(argc, argv);
		if(argman.size() < 5) {
			return use(1);
		}

		int host_index=argman.find_index("-h");
		if(-1==host_index) {
			return use(2);
		}

		int port_index=argman.find_index("-p");
		if(-1==port_index) {
			return use(2);
		}

		int port=std::atoi(argman.get_argument(port_index+1).c_str());
		if(0==port) {
			throw std::runtime_error("Invalid port");
		}

		bool with_ssl=-1!=argman.find_index("-ssl");

		app::client cl(argman.get_argument(host_index+1), port, with_ssl);

		//Get the welcome message... absolutely terrible, but well...		
		std::cout<<"Waiting for server welcome..."<<std::endl;
		std::cout<<cl.wait_for_answer();
		
		//Clearing input stream...
//		if(std::cin.rdbuf()->in_avail()) {
//			std::cin.ignore(std::numeric_limits<std::streamsize>::max());
//		}

		//And enter the endless loop.
		std::string line;
		while(!cl.is_done()) {
			std::cout<<">>";
			std::getline(std::cin, line);

			cl.send_message(line+'\n');
			std::cout<<cl.wait_for_answer()<<std::endl;
		}
		
		return 0;
	}
	catch(std::exception &e) {

		std::cout<<"Something failed: "<<e.what()<<std::endl;
		return -1;
	}
}