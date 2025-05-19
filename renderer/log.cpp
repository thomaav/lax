#include <stdarg.h>
#include <stdio.h>
#include <string>

#include <renderer/log.h>

loggerp *logger::m_logger = nullptr;

void logger::register_logger(loggerp *logger)
{
	m_logger = logger;
}

void logger::info(const char *str, ...)
{
	if (nullptr != m_logger)
	{
		std::string info_str = std::string("[INFO]: ") + std::string(str);
		char buffer[1024];
		va_list arglist;
		{
			va_start(arglist, str);
			vsnprintf(buffer, sizeof(buffer), info_str.data(), arglist);
			va_end(arglist);
		}
		logger::m_logger->log(buffer);
	}
}

void logger::warn(const char *str, ...)
{
	if (nullptr != m_logger)
	{
		std::string info_str = std::string("[WARNING]: ") + std::string(str);
		char buffer[1024];
		va_list arglist;
		{
			va_start(arglist, str);
			vsnprintf(buffer, sizeof(buffer), info_str.data(), arglist);
			va_end(arglist);
		}
		logger::m_logger->log(buffer);
	}
}

void logger::error(const char *str, ...)
{
	if (nullptr != m_logger)
	{
		std::string info_str = std::string("[ERROR]: ") + std::string(str);
		char buffer[1024];
		va_list arglist;
		{
			va_start(arglist, str);
			vsnprintf(buffer, sizeof(buffer), info_str.data(), arglist);
			va_end(arglist);
		}
		logger::m_logger->log(buffer);
	}
}

void console_logger::log(const char *str)
{
	printf("%s\n", str);
}
