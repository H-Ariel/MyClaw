#pragma once

#include "framework.h"
#include "UITextElement.h"


class WindowManager
{
public:
	static void Initialize(const TCHAR WindowClassName[], void* lpParam);
	static void Finalize();

	static void setTitle(const string& title) { SetWindowText(_hWnd, wstring(title.begin(), title.end()).c_str()); }
	static void setBackgroundColor(ColorF bgColor) { _backgroundColor = bgColor; }
	static ColorF getBackgroundColor() { return _backgroundColor; }
	static void setWindowOffset(const D2D1_POINT_2F* offset);

	static D2D1_SIZE_F getSize() { return { realSize.width / PixelSize, realSize.height / PixelSize }; } // get screen size and consider PixelSize
	static D2D1_SIZE_F getRealSize() { return realSize; }
	static const HWND& getHwnd() { return _hWnd; }

	static bool isInScreen(Rectangle2D rc); // return if `rc` is in the window area

	static void BeginDraw() { _renderTarget->BeginDraw(); _renderTarget->Clear(_backgroundColor); }
	static void EndDraw() { _renderTarget->EndDraw(); }

	static void resizeRenderTarget(D2D1_SIZE_U newSize);

	static void drawRect(Rectangle2D dst, D2D1_COLOR_F color, float width = 1);
	static void drawRect(Rectangle2D dst, ColorF color, float width = 1);
	static void fillRect(Rectangle2D dst, D2D1_COLOR_F color);
	static void fillRect(Rectangle2D dst, ColorF color);
	static void drawBitmap(ID2D1Bitmap* bitmap, Rectangle2D dst, bool mirrored);
	static void drawText(const wstring& text, IDWriteTextFormat* textFormat, ID2D1SolidColorBrush* brush, const Rectangle2D& layoutRect);
	static void drawText(const wstring& text, const FontData& font, const Rectangle2D& layoutRect, ColorF color);

	static ID2D1Bitmap* createBitmapFromBuffer(const void* const buffer, uint32_t width, uint32_t height);
	static IDWriteTextFormat* createTextFormat(const FontData& font);
	static ID2D1SolidColorBrush* createSolidBrush(ColorF color);
	

	static float PixelSize;

private:
	static bool _isInScreen(Rectangle2D& rc); // return if `rc` is in the window area and subtracts the window-offset from it


	static HWND _hWnd;
	static ID2D1Factory* _d2dFactory;
	static IDWriteFactory* _dWriteFactory;
	static ID2D1HwndRenderTarget* _renderTarget;
	static IWICImagingFactory* _wicImagingFactory;
	static const D2D1_POINT_2F* _windowOffset;
	static ColorF _backgroundColor;
	static D2D1_SIZE_F realSize;
};
