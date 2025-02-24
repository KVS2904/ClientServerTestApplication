#include "server.h"
#include <iostream>
#include <mutex>
#include <fstream>
#include <format>
#include "../utils/logger.h"

Server::Server(boost::asio::io_context& io_context, short port, int dump_interval)
	: io_context_(io_context), acceptor_(io_context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
	  dump_timer_(io_context), dump_interval_(dump_interval)
{
	start_accept();
	start_dump_timer();
}

void Server::start_accept()
{
	if (stopped_) return;
	auto new_socket = std::make_shared<boost::asio::ip::tcp::socket>(io_context_);

	acceptor_.async_accept(*new_socket,
		[this, new_socket](boost::system::error_code ec)
		{
			if (!ec)
			{
				Logger::log("Client connected!");
				start_read(new_socket);
			}

			if (!stopped_)
			{
				start_accept();
			}
		});
}

void Server::start_read(std::shared_ptr<boost::asio::ip::tcp::socket> socket)
{
	if (stopped_) return;
	auto buffer = std::make_shared<int>();

	boost::asio::async_read(*socket, boost::asio::buffer(buffer.get(), sizeof(int)),
		[this, socket, buffer](boost::system::error_code ec, std::size_t length)
		{
			if (!ec)
			{
				int received_number = *buffer;
				{
					std::lock_guard<std::mutex> lock(numbers_mutex_);
					unique_numbers_.insert(received_number).second;
				}

				double average = calculate_average();
				boost::asio::async_write(*socket, boost::asio::buffer(&average, sizeof(double)),
					[this, socket, average](boost::system::error_code ec, std::size_t length)
					{
						if (!ec)
						{
							Logger::log("Server sent average value to the client: " + std::format("{:.1f}", average));
						}

						if (!stopped_)
						{
							start_read(socket);
						}
					});
			}
		});
}

double Server::calculate_average()
{
	std::lock_guard<std::mutex> lock(numbers_mutex_);

	if (unique_numbers_.empty())
	{
		return 0.0;
	}

	double sum_of_squares = 0.0;
	for (int number : unique_numbers_)
	{
		sum_of_squares += static_cast<double>(number) * number;
	}

	return sum_of_squares / unique_numbers_.size();
}

	void Server::start_dump_timer()
{
	dump_timer_.expires_after(std::chrono::seconds(dump_interval_));
	dump_timer_.async_wait(
		[this](const boost::system::error_code& ec)
		{
			if (!ec)
			{
				std::thread dump_thread(&Server::dump_numbers, this);
				dump_thread.detach();

				start_dump_timer();
			}
		});
}

void Server::dump_numbers()
{
	std::lock_guard<std::mutex> lock(numbers_mutex_);

	std::ofstream outfile("dump.bin", std::ios::binary | std::ios::out);
	if (!outfile)
	{
		std::cerr << "Failed to open dump file!" << std::endl;
		return;
	}

	for (int number : unique_numbers_)
	{
		outfile.write(reinterpret_cast<const char*>(&number), sizeof(number));
	}

	Logger::log("Dump completed. Saved " + std::to_string(unique_numbers_.size()) + " numbers to dump.bin.");
}

void Server::stop()
{
	dump_timer_.cancel();

	if (acceptor_.is_open())
	{
		acceptor_.close();
	}
	stopped_ = true;
	Logger::log("Server stopped.");
}