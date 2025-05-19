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

/* (TODO, thoave01): Belongs in the editor somehow. */
class console_logger : public loggerp
{
public:
	console_logger() = default;
	~console_logger() = default;

	console_logger(const console_logger &) = delete;
	console_logger operator=(const console_logger &) = delete;

	void log(const char *str) override;
};
