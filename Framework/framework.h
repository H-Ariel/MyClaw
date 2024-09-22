#pragma once

#define _CRT_SECURE_NO_WARNINGS
#define _USE_MATH_DEFINES
#define NOMINMAX // use std::min/max instead of min/max macros


#ifdef _DEBUG
// check for memory leaks
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DBG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
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
#include <cmath>
#include <algorithm>
#include <functional>
#include <chrono>
#include <thread>
#include <mutex>
#include <vector>
#include <list>
#include <map>
#include <stack>
#include <set>
#include <queue>

#include "Exception.hpp"
#include "LogFile.h"


#pragma comment(lib, "d2d1")
#pragma comment(lib, "dwrite")
#pragma comment(lib, "winmm.lib")


#ifndef HINST_THISCOMPONENT
EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define HINST_THISCOMPONENT ((HINSTANCE)&__ImageBase)
#endif


using namespace std;
using namespace D2D1;


// replace '\' with '/' and remove the first '/' if exists
string fixPath(string path);

// check if `str` is starts with `prefix`
bool startsWith(const string& str, const string& prefix);

// check if `str` is ends with `suffix`
bool endsWith(const string& str, const string& suffix);

// check is `str1` contains `str2`
bool contains(const string& str1, const string& str2);

// reterns a random number in range [a,b]
float getRandomFloat(float a, float b);
int getRandomInt(int a, int b);

// make a DWORD from two WORDs
DWORD make_dword(WORD hi, WORD lo);


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

// my shell for `memcpy`. It can also accept `const` values.
template <class T>
inline void myMemCpy(const T& dst, const T& src) { memcpy((void*)&dst, &src, sizeof(T)); /* Hmmm... I am a hacker */ }

// check if `p`'s type is `Type`
template<typename Type, typename V>
inline bool isinstance(const V* p) { return dynamic_cast<const Type*>(p) != nullptr; }
