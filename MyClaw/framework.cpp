#include "framework.h"


string replaceString(string str, char src, char dst)
{
	replace(str.begin(), str.end(), src, dst);
	return str;
}

bool startsWith(string str, string prefix)
{
	return str.length() >= prefix.length() && 
		!strncmp(str.c_str(), prefix.c_str(), prefix.length());
}

bool endsWith(string str, string suffix)
{
	const size_t delta_len = str.length() - suffix.length();
	return delta_len >= 0 && !strcmp(str.c_str() + delta_len, suffix.c_str());
}

bool operator!=(D2D1_RECT_F a, D2D1_RECT_F b)
{
	return a.left != b.left || a.top != b.top || a.right != b.right || a.bottom != b.bottom;
}
