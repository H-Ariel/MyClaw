#pragma once

#include <stdio.h>
#include <mutex>


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

private:
	// log without lock mutex or check if file is open.
	static void unsafe_log(LogType type, const char* format, va_list args);

	static FILE* logFile;
	static std::mutex logMutex;
};
