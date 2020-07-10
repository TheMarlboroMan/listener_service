#include "client.h"

#include <chrono>
#include <thread>

using namespace app;

client::client(const std::string& _host, int _port, bool _secure)
	:client_instance{
		_port==-1
			? sck::client(_host)
			: sck::client(_host, _port, _secure)
	}{

}

std::string client::wait_for_answer() {

	while(true && client_instance.is_connected()) {

		std::string message=client_instance.receive(true);
		if(message.size()) {
			return message;

		}
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}

	return "finished / disconnected";
}
