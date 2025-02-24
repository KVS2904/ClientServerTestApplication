#include <boost/asio.hpp>
#include <iostream>
#include <thread>
#include "server/server.h"
#include "client/client.h"
#include "utils/logger.h"
#include <conio.h>

bool is_escape_pressed();
int main()
{
	Logger::init("server.log");
	try
	{
		boost::asio::io_context io_context;

		const short server_port = 12345;
		const int dump_interval = 10;
		Server server(io_context, server_port, dump_interval);

		const int num_clients = 10;
		const std::string host = "127.0.0.1";
		std::vector<std::shared_ptr<Client>> clients;
		std::vector<std::thread> client_threads;
		for (int i = 0; i < num_clients; ++i)
		{
			client_threads.emplace_back(
				[&io_context, host, server_port, i, &clients]()
				{
					auto client = std::make_shared<Client>(io_context, host, server_port, i);
					clients.push_back(client);
					io_context.run();
				});

			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}

        while (!is_escape_pressed())
		{
			std::this_thread::sleep_for(
				std::chrono::milliseconds(100));
		}

		Logger::log("ESC pressed. Shutting down...");
		

		for (auto& client : clients)
		{
			client->stop();
		}
		server.stop();
		io_context.stop();

		for (auto& thread : client_threads)
		{
			thread.join();
		}
	}
	catch (const std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << std::endl;
	}
	return 0;
}

bool is_escape_pressed()
{
	return _kbhit() && _getch() == 27;
}