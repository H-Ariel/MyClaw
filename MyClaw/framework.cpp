#include "framework.h"


string replaceString(string str, char src, char dst)
{
	replace(str.begin(), str.end(), src, dst);
	return str;
}

bool isNumber(string str)
{
	for (const char& c : str)
	{
		if (!isdigit(c))
		{
			return false;
		}
	}
	return true;
}

bool startsWith(string str, string prefix)
{
	return str.length() >= prefix.length() && !strncmp(str.c_str(), prefix.c_str(), prefix.length());
}

bool endsWith(string str, string suffix)
{
	const size_t delta_len = str.length() - suffix.length();
	return delta_len >= 0 && !strcmp(str.c_str() + delta_len, suffix.c_str());
}

bool contains(string str1, string str2)
{
	return str1.find(str2) != string::npos;
}

vector<string> splitStringIntoTokens(string input, char delim)
{
	stringstream sstr(input);
	string tok;
	vector<string> toksList;

	while (getline(sstr, tok, delim))
	{
		if (!tok.empty())
		{
			toksList.push_back(tok);
		}
	}

	return toksList;
}

float getRandomFloat(float a, float b)
{
	return (float)rand() / RAND_MAX * (b - a) + a;
}

int getRandomInt(int a, int b)
{
	return rand() % (b - a + 1) + a;
}

bool operator!=(D2D1_RECT_F a, D2D1_RECT_F b)
{
	return a.left != b.left || a.top != b.top || a.right != b.right || a.bottom != b.bottom;
}
