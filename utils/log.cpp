#include <stdarg.h>
#include <string>

#include "log.h"

loggerp *logger::m_logger = nullptr;

void logger::register_logger(loggerp *logger)
{
	m_logger = logger;
}

void logger::info(const char *str, ...)
{
	std::string info_str = std::string("[INFO]: ") + std::string(str);
	char buffer[1024];
	va_list arglist;
	{
		va_start(arglist, str);
		vsnprintf(buffer, sizeof(buffer), info_str.data(), arglist);
		va_end(arglist);
	}

	if (nullptr != m_logger)
	{
		logger::m_logger->log(buffer);
	}
	else
	{
		printf("%s\n", buffer);
	}
}

void logger::warn(const char *str, ...)
{
	std::string info_str = std::string("[WARNING]: ") + std::string(str);
	char buffer[1024];
	va_list arglist;
	{
		va_start(arglist, str);
		vsnprintf(buffer, sizeof(buffer), info_str.data(), arglist);
		va_end(arglist);
	}

	if (nullptr != m_logger)
	{
		logger::m_logger->log(buffer);
	}
	else
	{
		printf("%s\n", buffer);
	}
}

void logger::error(const char *str, ...)
{
	std::string info_str = std::string("[ERROR]: ") + std::string(str);
	char buffer[1024];
	va_list arglist;
	{
		va_start(arglist, str);
		vsnprintf(buffer, sizeof(buffer), info_str.data(), arglist);
		va_end(arglist);
	}

	if (nullptr != m_logger)
	{
		logger::m_logger->log(buffer);
	}
	else
	{
		printf("%s\n", buffer);
	}
}
