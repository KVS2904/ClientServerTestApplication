#include "client.h"
#include <iostream>
#include "../utils/logger.h"

Client::Client(boost::asio::io_context& io_context, const std::string& host, short port, int client_id)
	: socket_(io_context), client_id_(client_id)
{
	std::srand(static_cast<unsigned int>(std::time(nullptr)) + client_id);

	boost::asio::ip::tcp::resolver resolver(io_context);
	auto endpoints = resolver.resolve(host, std::to_string(port));
	boost::asio::async_connect(socket_, endpoints,
		[this](const boost::system::error_code& ec, const boost::asio::ip::tcp::endpoint& endpoint)
		{
			if (!ec)
			{
				Logger::log("Client " + std::to_string(client_id_) + " connected to the server!");
				start_communication_loop();
			}
		});
}

void Client::start_communication_loop()
{
	if (stopped_) return;
	int random_number = std::rand() % 1024;
	boost::asio::async_write(socket_, boost::asio::buffer(&random_number, sizeof(random_number)),
		[this, random_number](const boost::system::error_code& ec, std::size_t length)
		{
			if (!ec)
			{
				Logger::log("Client " + std::to_string(client_id_) + " sent number to the server: " + std::to_string(random_number));
				start_receiving();
			}
		});
}

void Client::start_receiving()
{
	auto buffer = std::make_shared<double>();

	boost::asio::async_read(socket_, boost::asio::buffer(buffer.get(), sizeof(double)),
		[this, buffer](const boost::system::error_code& ec, std::size_t length)
		{
			if (!ec)
			{
				double average = *buffer;
				std::this_thread::sleep_for(std::chrono::seconds(1));
				start_communication_loop();
			}
		});
}

void Client::stop()
{
	if (socket_.is_open())
	{
		socket_.close();
	}
	stopped_ = true;
	Logger::log("Client " + std::to_string(client_id_) + " stopped.");
}