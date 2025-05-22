#pragma once

class loggerp
{
public:
	virtual ~loggerp() = default;
	virtual void log(const char *str) = 0;
};

class logger
{
public:
	friend class loggerp;

	static void register_logger(loggerp *logger);

	static void info(const char *str, ...);
	static void warn(const char *str, ...);
	static void error(const char *str, ...);

private:
	static loggerp *m_logger;
};
