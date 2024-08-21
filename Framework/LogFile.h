#pragma once

#include <fstream>
#include <string>


class LogFile {
public:
	enum LogType {
		Error,
		Warning,
		Info
	};

	static void Initialize(const std::string& filename);
	static void Finalize();
	
	// Logs a message with a specific log type
	static void log(LogType type, const std::string& message);
	// Logs a formatted message with a specific log type
	static void logf(LogType type, const char* format, ...);

    
private:
	static std::string getCurrentTime();
	static const char* logTypeToString(LogType type);
	// Formats a string using a printf-style format and variable arguments
	static std::string vformat(const char* format, va_list args);

	static std::ofstream logStream;
};
