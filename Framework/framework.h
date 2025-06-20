/*
 * This header file provides essential utilities and debugging
 * support for the project, including many useful and necessary
 * components for almost every file in the codebase.
 */

#pragma once

#define _CRT_SECURE_NO_WARNINGS
#define _USE_MATH_DEFINES


#ifdef _DEBUG
// check for memory leaks
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DBG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#else
#define DBG_NEW new // do not check for memory leaks
#endif


#include <iostream>
#include <fstream>
#include <cmath>
#include <algorithm>
#include <functional>
#include <chrono>
#include <thread>
#include <mutex>
#include <vector>
#include <list>
#include <map>
#include <unordered_map>
#include <stack>

#include "Exception.hpp"


#ifdef _DEBUG
#define LOG printf
#else
#define LOG(...)
#endif


using namespace std;


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
