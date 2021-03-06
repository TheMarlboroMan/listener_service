#include "src/client.h"

#include <tools/arg_manager.h>

#include <signal.h>

#include <iostream>
#include <cstdio>
#include <limits>



//Test client with netcat 127.0.1.1 16666 or echo "HI" | netcat 127.0.0.1 16666
int use(int _v);
void handle_sigint(int _s);

int main(int argc, char ** argv) {

	try {
		signal(SIGINT, handle_sigint);

		tools::arg_manager argman(argc, argv);
		if(argman.size() < 5) {
			return use(1);
		}

		int host_index=argman.find_index("-h");
		int port_index=argman.find_index("-p");

		if(-1==port_index && -1==host_index) {
			return use(2);
		}

		int port=0;
		if(-1!=port_index) {
			int argport=std::atoi(argman.get_argument(port_index+1).c_str());
			if(0==argport) {
				throw std::runtime_error("Invalid port");
			}
			port=argport;
		}

		bool with_ssl=-1!=argman.find_index("-ssl");

		app::client cl(argman.get_argument(host_index+1), port, with_ssl);

		//Get the welcome message... absolutely terrible, but well...
		std::cout<<"Waiting for server welcome..."<<std::endl;
		std::cout<<cl.wait_for_answer();

		//And wait for the security
		std::cout<<cl.wait_for_answer();

		//Clearing input stream...
//		if(std::cin.rdbuf()->in_avail()) {
//			std::cin.ignore(std::numeric_limits<std::streamsize>::max());
//		}

		//And enter the endless loop.
		std::string line;
		while(!cl.is_done()) {

			//TODO: Thing is, this is blocking...
			std::cout<<">>";
			std::getline(std::cin, line);
			cl.send(line+'\n');
			std::cout<<cl.wait_for_answer()<<std::endl;
		}

		return 0;
	}
	catch(std::exception &e) {

		std::cout<<"Something failed: "<<e.what()<<std::endl;
		return -1;
	}
}

int use(int _v) {

	std::cout<<"["<<_v<<"] use: ./client.out -h #host -p #port [-ssl]\n"
"\t-h Host name | unix filename\n"
"\t-p Port number, -1 for local socket\n"
"\t-ssl Enable SSL"<<std::endl;

	return _v;
}

void handle_sigint(int) {

	exit(1);
}
