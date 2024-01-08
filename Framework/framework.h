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

	#define LOW_DETAILS		// avoid usage of treasures glitter, front plane, sounds, some objects, etc.
#else
	#define DBG_NEW new // do not check for memory leaks
#endif


#include <SDKDDKVer.h>
#include <windows.h>
#include <d2d1.h>
#include <dwrite.h>
#include <d2d1helper.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <list>
#include <map>
#include <cmath>
#include <functional>
#include <algorithm>
#include <thread>
#include <mutex>
#include <stack>
#include <set>

#include "Exception.hpp"


#pragma comment(lib, "d2d1")
#pragma comment(lib, "dwrite")
#pragma comment(lib, "winmm.lib")


#ifndef HINST_THISCOMPONENT
EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define HINST_THISCOMPONENT ((HINSTANCE)&__ImageBase)
#endif


#ifdef _DEBUG
	#define DBG_PRINT(...) printf(__VA_ARGS__)
#else
	#define DBG_PRINT(...)
#endif


using namespace std;
using namespace D2D1;


// replace `src` with `dst` in `str`
string replaceString(string str, char src, char dst);

// check if `str` is starts with `prefix`
bool startsWith(const string& str, const string& prefix);

// check if `str` is ends with `suffix`
bool endsWith(const string& str, const string& suffix);

// check is `str1` contains `str2`
bool contains(const string& str1, const string& str2);

// reterns a random number in range [a,b]
float getRandomFloat(float a, float b);
int getRandomInt(int a, int b);


// reverses the bytes order of `t`
template <class T>
inline T reverseBytes(T t)
{
	uint8_t* ptr = (uint8_t*)(&t);
	std::reverse(ptr, ptr + sizeof(T));
	return t;
}

// check if `arr` contains `val`
template <class ArrT, class ValT>
inline bool FindInArray(ArrT arr, ValT val)
{
	auto arrEnd = end(arr);
	return find(begin(arr), arrEnd, val) != arrEnd;
}

// insert `v2` to end of `v1`
template <class T>
inline vector<T> operator+(const vector<T>& v1, const vector<T>& v2)
{
	vector<T> v = v1;
	v.insert(v.end(), v2.begin(), v2.end());
	return v;
}

// append `v2` to end of `v1`
template <class T>
inline vector<T>& operator+=(vector<T>& v1, const vector<T>& v2)
{
	v1 = v1 + v2;
	return v1;
}

// safe release for COM objects
template <class T>
inline void SafeRelease(T*& p)
{
	if (p)
	{
		p->Release();
		p = nullptr;
	}
}
// safe delete for regular objects
template <class T>
inline void SafeDelete(T*& p)
{
	if (p)
	{
		delete p;
		p = nullptr;
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
