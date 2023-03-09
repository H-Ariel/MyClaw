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
	#define CHECK_FOR_MEMORY_LEAKS

//	#define DRAW_RECTANGLES	// draw rectangles around tiles, objects, characters, etc.
	#define COUNT_FPS		// show FPS as window title
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
#include <string>
#include <map>
#include <cmath>
#include <functional>
#include <algorithm>
#include <thread>
#include <mutex>
#include <stack>
#include <set>

#include "Exception.hpp"
#include "CollisionDistances.h"
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


struct ColorRGBA
{
	uint8_t r, g, b, a;
};


using namespace std;
using namespace D2D1;


// replace `src` with `dst` in `str`
string replaceString(string str, char src, char dst);

// check if `str` is number as string
bool isNumber(string str);

// check if `str` is starts with `prefix`
bool startsWith(string str, string prefix);
// check if `str` is ends with `suffix`
bool endsWith(string str, string suffix);
// check is `str1` contains `str2`
bool contains(string str1, string str2);

// split `str` by `delim`
vector<string> splitStringIntoTokens(string str, char delim);

// reterns a random number in range [a,b]
float getRandomFloat(float a, float b);
int getRandomInt(int a, int b);

bool operator!=(D2D1_RECT_F a, D2D1_RECT_F b);


// check if `arr` contains `val`
template <class ArrT, class ValT>
inline bool FindInArray(ArrT arr, ValT val)
{
	auto arrEnd = end(arr);
	return find(begin(arr), arrEnd, val) != arrEnd;
}

// safe release for COM objects
template <class T>
void SafeRelease(T** ppT)
{
	if (*ppT)
	{
		(*ppT)->Release();
		*ppT = nullptr;
	}
}

// reverses the bytes order of `obj`
template <class T>
inline void reverseBytes(T& obj)
{
	uint8_t* ptr = (uint8_t*)(&obj);
	reverse(ptr, ptr + sizeof(T));
}

// allocate new object as `shared_ptr`
template <class T, class ... Args>
shared_ptr<T> allocNewSharedPtr(Args... args)
{
	return shared_ptr<T>(DBG_NEW T(args...));
}

// my shell for `memcpy`. It can also accept `const` values.
template <class T>
inline void myMemCpy(const T& dst, const T& src)
{
	memcpy((void*)&dst, &src, sizeof(T));
	// Hmmm... I am a hacker XD
}

// check if `p`'s type is `Type`
template<typename Type, typename V>
inline bool isinstance(const V* p)
{
	return typeid(*p) == typeid(Type);
}

// check if `p`'s base type is `Base`
template<typename Base, typename V>
inline bool isbaseinstance(const V* p)
{
	return dynamic_cast<const Base*>(p) != nullptr;
}
