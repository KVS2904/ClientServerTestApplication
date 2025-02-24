#pragma once
#include <boost/asio.hpp>

class Client
{
public:
	Client(boost::asio::io_context& io_context, const std::string& host, short port, int client_id);
	void stop();

private:
	boost::asio::ip::tcp::socket socket_;
	int client_id_;
	bool stopped_ = false;

	void start_communication_loop();
	void start_receiving();
};