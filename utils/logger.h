#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <mutex>

class Logger
{
public:
	static void log(const std::string& message);
	static void init(const std::string& filename);

private:
	static std::ofstream log_file;
	static std::mutex log_mutex;
};