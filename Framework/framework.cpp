#include "Framework.h"


string fixPath(string path)
{
	for (char& c : path)
		if (c == '\\') c = '/';

	if (path[0] == '/')
		path = path.substr(1);

	return path;
}

bool startsWith(const string& str, const string& prefix)
{
	return str.length() >= prefix.length() && !strncmp(str.c_str(), prefix.c_str(), prefix.length());
}

bool endsWith(const string& str, const string& suffix)
{
	const size_t delta_len = str.length() - suffix.length();
	return delta_len >= 0 && !strcmp(str.c_str() + delta_len, suffix.c_str());
}

bool contains(const string& str1, const string& str2)
{
	return str1.find(str2) != string::npos;
}

float getRandomFloat(float a, float b)
{
	return (float)rand() / RAND_MAX * (b - a) + a;
}

int getRandomInt(int a, int b)
{
	return rand() % (b - a + 1) + a;
}
