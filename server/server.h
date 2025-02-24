#pragma once
#include <boost/asio.hpp>
#include <set>

class Server
{
public:
	Server(boost::asio::io_context& io_context, short port, int dump_interval);
	void stop();

private:
	boost::asio::io_context& io_context_;
	boost::asio::ip::tcp::acceptor acceptor_;
	boost::asio::steady_timer dump_timer_;
	std::set<int> unique_numbers_;
	std::mutex numbers_mutex_;
	int dump_interval_;
	bool stopped_ = false;

	void start_accept();
	void start_read(std::shared_ptr<boost::asio::ip::tcp::socket> socket);
	double calculate_average();
	void start_dump_timer();
	void dump_numbers();
};