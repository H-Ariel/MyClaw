#pragma once

#include "framework.h"


class WindowManager
{
public:
	static void Initialize(const TCHAR WindowClassName[], void* lpParam);
	static void Finalize();

	static void setTitle(string title);
	static void setTitle(wstring title);
	static void resizeRenderTarget(D2D1_SIZE_U newSize);
	static D2D1_SIZE_F getSize(); // get screen size and consider PixelSize
	static D2D1_SIZE_F getRealSize();
	static void setSize(D2D1_SIZE_F size);
	static HWND getHwnd();

	static void drawRect(D2D1_RECT_F dst, D2D1_COLOR_F color, float width = 1);
	static void drawRect(D2D1_RECT_F dst, ColorF color, float width = 1);
	static void drawBitmap(ID2D1Bitmap* bitmap, D2D1_RECT_F dst, bool mirrored);

	static bool isInScreen(D2D1_RECT_F rc); // return if `rc` is in the window area
	static void setWindowOffset(const D2D1_POINT_2F* offset);


	static float PixelSize;

private:
	static bool _isInScreen(D2D1_RECT_F& rc); // return if `rc` is in the window area and subtracts the window-offset from it


	static HWND _hWnd;
	static ID2D1Factory* _d2dFactory;
	static ID2D1HwndRenderTarget* _renderTarget;
	static IWICImagingFactory* _wicImagingFactory;
	static const D2D1_POINT_2F* _windowOffset;


	friend class BaseEngine;
	friend class ImagesManager;
};
