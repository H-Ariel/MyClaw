#include "Framework.h"


// TODO: use C code here (FILE*, fprintf, etc.). The overhead of C++ is too high


// Definition of the static member
ofstream LogFile::logStream;

void LogFile::Initialize(const string& filename) {
	logStream.open(filename, ios::out | ios::app);
	if (!logStream) {
		throw Exception("Failed to open log file");
	}
	log(Info, "Good morning!");
}

void LogFile::Finalize() {
	if (logStream.is_open()) {
		log(Info, "Good night :)");
		logStream.close();
	}
}

void LogFile::log(LogType type, const string& message) {
	if (logStream.is_open()) {
		logStream << getCurrentTime() << " [" << logTypeToString(type) << "] " << message << endl;
	}
}

void LogFile::logf(LogType type, const char* format, ...) {
	if (logStream.is_open()) {
		va_list args;
		va_start(args, format);
		string formattedMessage = vformat(format, args);
		va_end(args);
		log(type, formattedMessage);
	}
}

string LogFile::getCurrentTime() {
	time_t now = time(nullptr);
	char buffer[20];
	strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", localtime(&now));
	return buffer;
}

const char* LogFile::logTypeToString(LogType type) {
	switch (type) {
	case Error:   return "ERROR";
	case Warning: return "WARNING";
	case Info:    return "INFO";
	default:      return "UNKNOWN";
	}
}

string LogFile::vformat(const char* format, va_list args) {
	va_list tmpArgs;
	va_copy(tmpArgs, args);
	const int len = vsnprintf(nullptr, 0, format, tmpArgs);
	va_end(tmpArgs);
	vector<char> buffer(len + 1);
	vsnprintf(buffer.data(), buffer.size(), format, args);
	return string(buffer.data(), len);
}
