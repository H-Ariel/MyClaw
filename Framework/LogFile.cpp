#include "Framework.h"

#ifdef _DEBUG

FILE* LogFile::logFile = nullptr;
mutex LogFile::logMutex;


void LogFile::Initialize(const char* filename) {
	lock_guard<mutex> lock(logMutex);

	if (logFile)
		throw Exception("LogFile is already initialized.");

	logFile = fopen(filename, "a");
	if (!logFile)
		throw Exception("Failed to open log file");

	unsafe_log(Info, "Logger initialized successfully.", nullptr);
}

void LogFile::Finalize() {
	lock_guard<mutex> lock(logMutex);
	if (logFile) {
		unsafe_log(Info, "Logger finalized successfully.", nullptr);
		fclose(logFile);
		logFile = nullptr;
	}
}

void LogFile::log(LogType type, const char* format, ...) {
	lock_guard<mutex> lock(logMutex);
	if (logFile) {
		va_list args;
		va_start(args, format);
		unsafe_log(type, format, args);
		va_end(args);
	}
}

void LogFile::unsafe_log(LogType type, const char* format, va_list args)
{
	// get time
	time_t now = time(nullptr);
	char timeBuffer[20];
	strftime(timeBuffer, sizeof(timeBuffer), "%Y-%m-%d %H:%M:%S", localtime(&now));

	// get type
	const char* typeStr = "UNKNOWN";
	switch (type) {
	case Error:   typeStr = "ERROR"; break;
	case Warning: typeStr = "WARNING"; break;
	case Info:    typeStr = "INFO"; break;
	}

	// log to file
	fprintf(logFile, "%s [%s] ", timeBuffer, typeStr);
	vfprintf(logFile, format, args);
	fprintf(logFile, "\n"); // To add a newline after the message if desired
}

#else

void LogFile::Initialize(const char* filename) {}
void LogFile::Finalize() {}
void LogFile::log(LogType type, const char* format, ...) {}

#endif
