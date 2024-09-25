#pragma once

#include <stdio.h>
#include <mutex>


// A simple class for logging messages to a file
// NOTE: This class log only in debug mode
class LogFile {
public:
	enum LogType {
		Error,
		Warning,
		Info
	};

	static void Initialize(const char* filename);
	static void Finalize();
	
	// Logs a formatted message with a specific log type
	static void log(LogType type, const char* format, ...);

#ifdef _DEBUG // these members use for logging, so we don't need them in release mode.
private:
	// log without lock mutex or check if file is open.
	static void unsafe_log(LogType type, const char* format, va_list args);

	static FILE* logFile;
	static std::mutex logMutex;
#endif
};
