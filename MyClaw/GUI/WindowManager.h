#pragma once

#include "../framework.h"
#include "UITextElement.h"


class WindowManager
{
public:
	static void Initialize(const TCHAR WindowClassName[], void* lpParam);
	static void Finalize();

	static void setTitle(const string& title);
	static void setWindowOffset(const D2D1_POINT_2F* offset);
	static void setBackgroundColor(ColorF bgColor);
	static ColorF getBackgroundColor();
	static D2D1_SIZE_F getSize();// get screen size and consider PixelSize
	static D2D1_SIZE_F getRealSize();
	static HWND getHwnd();

	static bool isInScreen(Rectangle2D rc); // return if `rc` is in the window area

	static void resizeRenderTarget(D2D1_SIZE_U newSize);
	static void BeginDraw();
	static void EndDraw();

	static void drawRect(Rectangle2D dst, ColorF color, float width = 1);
	static void fillRect(Rectangle2D dst, ColorF color);
	static void drawHole(D2D1_POINT_2F center, float radius, ColorF color); // draw around the circle with `color` and fill the circle with transparent
	static void drawBitmap(ID2D1Bitmap* bitmap, Rectangle2D dst, bool mirrored, float opacity = 1);
	static void drawText(const wstring& text, IDWriteTextFormat* textFormat, ColorF color, const Rectangle2D& layoutRect);
	static void drawText(const wstring& text, const FontData& font, ColorF color, const Rectangle2D& layoutRect);

	// create D2D1 objects.
	static ID2D1SolidColorBrush* getBrush(ColorF color);
	// NOTE: you need to release them manually
	static ID2D1Bitmap* createBitmapFromBuffer(const void* const buffer, uint32_t width, uint32_t height);
	static IDWriteTextFormat* createTextFormat(const FontData& font);
	

	static float PixelSize;
	static const D2D1_SIZE_F DEFAULT_WINDOW_SIZE;

private:
	static void drawRect(Rectangle2D dst, D2D1_COLOR_F color, float width);
	static void fillRect(Rectangle2D dst, D2D1_COLOR_F color);
	static ID2D1SolidColorBrush* getBrush(D2D1_COLOR_F color);

	static bool _isInScreen(Rectangle2D& rc); // return if `rc` is in the window area and subtracts the window-offset from it

	static WindowManager* instance;

	WindowManager(const TCHAR WindowClassName[], void* lpParam);
	~WindowManager();


	map<ColorF, ID2D1SolidColorBrush*> brushes; // cache brushes
	HWND _hWnd;
	ID2D1Factory* _d2dFactory;
	IDWriteFactory* _dWriteFactory;
	ID2D1HwndRenderTarget* _renderTarget;
	IWICImagingFactory* _wicImagingFactory;
	const D2D1_POINT_2F* _windowOffset;
	ColorF _backgroundColor;
	D2D1_SIZE_F realSize;
};
