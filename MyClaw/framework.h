#pragma once

#define _CRT_SECURE_NO_WARNINGS
#define _USE_MATH_DEFINES
#define NOMINMAX


#ifdef _DEBUG
	// check for memory leaks
	#define _CRTDBG_MAP_ALLOC
	#include <stdlib.h>
	#include <crtdbg.h>
	#define DBG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)

	#define LOW_DETAILS		// cancel the `...Candy` objects, treasures glitter, front plane, sounds, etc.
#else
	#define DBG_NEW new // do not check for memory leaks
#endif


#include <SDKDDKVer.h>
#include <windows.h>
#include <d2d1.h>
#include <d2d1helper.h>
#include <wincodec.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <cmath>
#include <functional>
#include <algorithm>
#include <thread>
#include <mutex>
#include <stack>
#include <set>

#include "Exception.hpp"
#include "Rectangle2D.h"
#include "PathManager.h"


#pragma comment(lib, "d2d1")
#pragma comment(lib, "windowscodecs.lib")
#pragma comment(lib, "winmm.lib")


#ifndef HINST_THISCOMPONENT
EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define HINST_THISCOMPONENT ((HINSTANCE)&__ImageBase)
#endif

// throw exception of the last winapi error (if it exists)
#define THROW_WINAPI_EXCEPTION { \
	int errId = GetLastError(); \
	if (errId != NO_ERROR) { \
		char buf[512]; \
		sprintf_s(buf, "WINAPI failed at:\nfile %s, line: %d, error id: %d", __FILE__, __LINE__, errId); \
		throw Exception(buf); \
	} \
}

// throw the last winapi error if `x` is `nullptr`
#define WINAPI_THROW_IF_NULL(x)		if ((x) == nullptr) THROW_WINAPI_EXCEPTION

// throw if `x` faild. `x` should be HRESULT object or function with HRESULT return type
#define HRESULT_THROW_IF_FAILED(x)	if (FAILED(x)) { \
	char buf[512]; \
	sprintf_s(buf, "HRESULT failed at:\nfile %s, line %d", __FILE__, __LINE__); \
	throw Exception(buf); \
}

// throw exception if the value of `var` is `nullptr`
#define THROW_IF_NULL(var)			if (var == nullptr) throw Exception(__FUNCTION__ " - " #var " = null");



enum class MouseButtons
{
	Left,
	Middle,
	Right
};


using namespace std;
using namespace D2D1;


// replace `src` with `dst` in `str`
inline string replaceString(string str, char src, char dst)
{
	replace(str.begin(), str.end(), src, dst);
	return str;
}

// check if `str` is starts with `prefix`
inline bool startsWith(const string& str, const string& prefix)
{
	return str.length() >= prefix.length() && !strncmp(str.c_str(), prefix.c_str(), prefix.length());
}
// check if `str` is ends with `suffix`
inline bool endsWith(const string& str, const string& suffix)
{
	const size_t delta_len = str.length() - suffix.length();
	return delta_len >= 0 && !strcmp(str.c_str() + delta_len, suffix.c_str());
}
// check is `str1` contains `str2`
inline bool contains(const string& str1, const string& str2) { return str1.find(str2) != string::npos; }

// reterns a random number in range [a,b]
inline float getRandomFloat(float a, float b) { return (float)rand() / RAND_MAX * (b - a) + a; }
inline int getRandomInt(int a, int b) { return rand() % (b - a + 1) + a; }


// check if `arr` contains `val`
template <class ArrT, class ValT>
inline bool FindInArray(ArrT arr, ValT val)
{
	auto arrEnd = end(arr);
	return find(begin(arr), arrEnd, val) != arrEnd;
}

// safe release for COM objects
template <class T>
inline void SafeRelease(T** ppT)
{
	if (*ppT)
	{
		(*ppT)->Release();
		*ppT = nullptr;
	}
}


// allocate new object as `shared_ptr`
template <class T, class ... Args>
inline shared_ptr<T> allocNewSharedPtr(Args... args) { return shared_ptr<T>(DBG_NEW T(args...)); }

// my shell for `memcpy`. It can also accept `const` values.
template <class T>
inline void myMemCpy(const T& dst, const T& src) { memcpy((void*)&dst, &src, sizeof(T)); /* Hmmm... I am a hacker */ }

// check if `p`'s type is `Type`
template<typename Type, typename V>
inline bool isinstance(const V* p) { return typeid(*p) == typeid(Type); }

// check if `p`'s base type is `Base`
template<typename Base, typename V>
inline bool isbaseinstance(const V* p) { return dynamic_cast<const Base*>(p) != nullptr; }
