#include "Logger.h"

// instantiated by InitGlobals
Logger* LOGGER = NULL;



Logger::Logger(const std::string& name) {
	const std::string stdinpLogNameP1 = name + "stdinp-p1.txt", stdinpLogNameP2 = name + "stdinp-p2.txt";
	const std::string stdoutLogNameP1 = name + "stdout-p1.txt", stdoutLogNameP2 = name + "stdout-p2.txt";
	const std::string stderrLogNameP1 = name + "stderr-p1.txt", stderrLogNameP2 = name + "stderr-p2.txt";

	#if (ENABLE_LOG_STDINP == 1)
	if ((stdinpLog = fopen(stdinpLogNameP1.c_str(), "r")) != NULL) {
		fclose(stdinpLog);
		stdinpLog = fopen(stdinpLogNameP2.c_str(), "w");
	} else {
		stdinpLog = fopen(stdinpLogNameP1.c_str(), "w");
	}
	#endif
	#if (ENABLE_LOG_STDOUT == 1)
	if ((stdoutLog = fopen(stdoutLogNameP1.c_str(), "r")) != NULL) {
		fclose(stdoutLog);
		stdoutLog = fopen(stdoutLogNameP2.c_str(), "w");
	} else {
		stdoutLog = fopen(stdoutLogNameP1.c_str(), "w");
	}
	#endif
	#if (ENABLE_LOG_STDERR == 1)
	if ((stderrLog = fopen(stderrLogNameP1.c_str(), "r")) != NULL) {
		fclose(stderrLog);
		stderrLog = fopen(stderrLogNameP2.c_str(), "w");
	} else {
		stderrLog = fopen(stderrLogNameP1.c_str(), "w");
	}
	#endif
}
Logger::~Logger() {
	#if (ENABLE_LOG_STDINP == 1)
	fclose(stdinpLog);
	#endif
	#if (ENABLE_LOG_STDOUT == 1)
	fclose(stdoutLog);
	#endif
	#if (ENABLE_LOG_STDERR == 1)
	fclose(stderrLog);
	#endif
}

FILE* Logger::GetLog(unsigned int mode) {
	switch (mode) {
		case LOGF_STDINP: { return stdinpLog; } break;
		case LOGF_STDOUT: { return stdoutLog; } break;
		case LOGF_STDERR: { return stderrLog; } break;
	}
	return NULL;
}
