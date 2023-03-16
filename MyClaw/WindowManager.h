#pragma once

#include "framework.h"


class WindowManager
{
public:
	static void Initialize(const TCHAR WindowClassName[], void* lpParam);
	static void Finalize();

	static void setTitle(string title) { setTitle(wstring(title.begin(), title.end())); }
	static void setTitle(wstring title) { SetWindowText(_hWnd, title.c_str()); }
	static void setSize(D2D1_SIZE_F size);
	static void setBackgroundColor(ColorF bgColor) { _backgroundColor = bgColor; }
	static ColorF getBackgroundColor() { return _backgroundColor; }
	static void setWindowOffset(const D2D1_POINT_2F* offset);

	static D2D1_SIZE_F getSize(); // get screen size and consider PixelSize
	static D2D1_SIZE_F getRealSize();
	static HWND getHwnd() { return _hWnd; }

	static void BeginDraw() { _renderTarget->BeginDraw(); _renderTarget->Clear(_backgroundColor); }
	static void EndDraw() { _renderTarget->EndDraw(); }

	static void resizeRenderTarget(D2D1_SIZE_U newSize);

	static void drawRect(D2D1_RECT_F dst, D2D1_COLOR_F color, float width = 1);
	static void drawRect(D2D1_RECT_F dst, ColorF color, float width = 1);
	static void fillRect(D2D1_RECT_F dst, D2D1_COLOR_F color);
	static void fillRect(D2D1_RECT_F dst, ColorF color);
	static void drawCircle(D2D1_POINT_2F center, FLOAT radius, ColorF color, float width = 1);
	static void drawBitmap(ID2D1Bitmap* bitmap, D2D1_RECT_F dst, bool mirrored);

	static ID2D1Bitmap* createBitmapFromBuffer(const void* const buffer, uint32_t width, uint32_t height);

	static bool isInScreen(D2D1_RECT_F rc); // return if `rc` is in the window area


	static float PixelSize;

private:
	static bool _isInScreen(D2D1_RECT_F& rc); // return if `rc` is in the window area and subtracts the window-offset from it
	static bool _isInScreen(D2D1_ELLIPSE& el); // return if `el` is in the window area and subtracts the window-offset from it


	static HWND _hWnd;
	static ID2D1Factory* _d2dFactory;
	static ID2D1HwndRenderTarget* _renderTarget;
	static IWICImagingFactory* _wicImagingFactory;
	static const D2D1_POINT_2F* _windowOffset;
	static ColorF _backgroundColor;
};
