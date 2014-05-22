#ifndef _LOGGER_HDR_
#define _LOGGER_HDR_

#include <string>

#ifndef DEBUG_LOG
#define ENABLE_LOG_STDINP 0
#define ENABLE_LOG_STDOUT 0
#define ENABLE_LOG_STDERR 0
#else
#define ENABLE_LOG_STDINP 1
#define ENABLE_LOG_STDOUT 1
#define ENABLE_LOG_STDERR 1
#endif

#if (ENABLE_LOG_STDINP == 1)
#define LOG_STDINP(L, ...)                                 \
	fprintf(L->GetLog(Logger::LOGF_STDINP), __VA_ARGS__);  \
	fflush(L->GetLog(Logger::LOGF_STDINP));
#else
#define LOG_STDINP(L, ...)
#endif

#if (ENABLE_LOG_STDOUT == 1)
#define LOG_STDOUT(L, ...)                                 \
	fprintf(L->GetLog(Logger::LOGF_STDOUT), __VA_ARGS__);  \
	fflush(L->GetLog(Logger::LOGF_STDOUT));
#else
#define LOG_STDOUT(L, ...)
#endif

#if (ENABLE_LOG_STDERR == 1)
#define LOG_STDERR(L, ...)                                 \
	fprintf(L->GetLog(Logger::LOGF_STDERR), __VA_ARGS__);  \
	fflush(L->GetLog(Logger::LOGF_STDERR));
#else
#define LOG_STDERR(L, ...)
#endif

class Logger {
public:
	enum {
		LOGF_STDINP = 0,
		LOGF_STDOUT = 1,
		LOGF_STDERR = 2,
	};

	Logger(const std::string&);
	~Logger();

	FILE* GetLog(unsigned int);

private:
	FILE* stdinpLog;
	FILE* stdoutLog;
	FILE* stderrLog;
};

extern Logger* LOGGER;

#endif
