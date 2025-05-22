#include <iomanip>
#include <sstream>
#include <string>

#include "log.h"

void console_logger::log(const char *str)
{
	auto t = time(nullptr);
	auto tm = *localtime(&t);
	std::ostringstream oss;
	oss << std::put_time(&tm, "[%H:%M:%S]");
	std::string console_str = oss.str() + std::string(str) + "\n";
	m_buffer.append(console_str.c_str());
}
