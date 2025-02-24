#include "logger.h"
#include <ctime>

std::ofstream Logger::log_file;
std::mutex Logger::log_mutex;

void Logger::init(const std::string& filename)
{
	std::lock_guard<std::mutex> lock(log_mutex);
	log_file.open(filename, std::ios::app);
	if (!log_file.is_open())
	{
		std::cerr << "Failed to open log file: " << filename << std::endl;
	}
}

void Logger::log(const std::string& message)
{
	std::lock_guard<std::mutex> lock(log_mutex);

	std::time_t now = std::time(nullptr);
	struct tm time_info;
	localtime_s(&time_info, &now);

	char time_buf[20];
	strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", &time_info);

	std::string log_message = "[" + std::string(time_buf) + "] " + message;

	std::cout << log_message << std::endl;

	if (log_file.is_open())
	{
		log_file << log_message << std::endl;
	}
}
