/*
 * This header file extends the previous framework by adding
 * Windows-specific functionality and libraries, including
 * Direct2D, DirectWrite, and DirectSound support.
 */

#pragma once

#define _CRT_SECURE_NO_WARNINGS
#define NOMINMAX // use std::min/max instead of min/max macros

#include <SDKDDKVer.h>
#include <windows.h>
#include <d2d1.h>
#include <dwrite.h>
#include <d2d1helper.h>
#include <dsound.h>

#include "Framework/Framework.h"
#include "Framework/DynamicArray.hpp"
#include "Rectangle2D.h"


#pragma comment(lib, "d2d1")
#pragma comment(lib, "dwrite")
#pragma comment(lib, "dsound.lib")
#pragma comment(lib, "winmm.lib")



#ifndef HINST_THISCOMPONENT
EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define HINST_THISCOMPONENT ((HINSTANCE)&__ImageBase)
#endif

// throw exception if `func` failed
#define TRY_HRESULT(func, msg) if (FAILED(func)) throw Exception(msg);


enum class MouseButtons
{
	Left,
	Middle,
	Right
};


using namespace D2D1;

