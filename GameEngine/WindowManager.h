#pragma once

#include "UITextElement.h"
#include "UIBaseImage.h"


class WindowManager
{
public:
	static void Initialize(const TCHAR WindowClassName[], const TCHAR title[], void* lpParam);
	static void Finalize();

	static void setTitle(const string& title);

	static void setWindowOffset(const D2D1_POINT_2F& offset) { myMemCpy(instance->_windowOffset, offset); }
	static void setBackgroundColor(ColorF bgColor) { instance->_backgroundColor = bgColor; }
	static ColorF getBackgroundColor() { return instance->_backgroundColor; }
	static D2D1_SIZE_F getCameraSize() { return instance->_camSize; };
	static D2D1_SIZE_F getRealSize() { return instance->_realSize; }
	static HWND getHwnd() { return instance->_hWnd; }

	static void setWindowScale(float scale);
	static float getWindowScale() { return instance->_windowScale; }
	static void setDefaultWindowScale(); // fit pixel size to default resolution (640x480)

	static bool isInScreen(Rectangle2D rc); // return if `rc` is in the window area

	static void resizeRenderTarget(D2D1_SIZE_U newSize);
	static void BeginDraw();
	static void EndDraw();
	static void Clear(); // fill window with background color

	static void drawRect(Rectangle2D dst, ColorF color, float width = 1);
	static void fillRect(Rectangle2D dst, ColorF color);
	static void drawBitmap(ID2D1Bitmap* bitmap, Rectangle2D dst, bool mirrored, bool upsideDown, float opacity);
	static void drawText(const wstring& text, IDWriteTextFormat* textFormat, ColorF color, const Rectangle2D& layoutRect);
	static void drawText(const wstring& text, const FontData& font, ColorF color, const Rectangle2D& layoutRect);

	static void drawHole(D2D1_POINT_2F center, float radius); // draw around the circle with `color` and fill the circle with transparent
	static void drawWrapCover(float top);

	// create D2D1 objects.
	static ID2D1SolidColorBrush* getBrush(ColorF color);
	static IDWriteTextFormat* createTextFormat(const FontData& font);

	static shared_ptr<UIBaseImage> createImage(const string& key, const void* const buffer, uint32_t width, uint32_t height, float offsetX, float offsetY);
	static shared_ptr<UIBaseImage> getImage(const string& key);
	static void clearImages(function <bool(const string&)> predicate);

	static const D2D1_SIZE_F DEFAULT_WINDOW_SIZE;

private:
	static void drawRect(Rectangle2D dst, D2D1_COLOR_F color, float width);
	static void fillRect(Rectangle2D dst, D2D1_COLOR_F color);
	static ID2D1SolidColorBrush* getBrush(D2D1_COLOR_F color);
	static bool _isInScreen(Rectangle2D& rc); // return if `rc` is in the window area and subtracts the window-offset from it

	static WindowManager* instance;

	WindowManager(const TCHAR WindowClassName[], const TCHAR title[], void* lpParam);
	~WindowManager();


	map<ColorF, ID2D1SolidColorBrush*> brushes; // cache brushes
	map<string, shared_ptr<UIBaseImage>> images; // cache bitmaps [key]=<img>
	HWND _hWnd;
	ID2D1Factory* _d2dFactory;
	IDWriteFactory* _dWriteFactory;
	ID2D1HwndRenderTarget* _renderTarget;
	const D2D1_POINT_2F _windowOffset;
	ColorF _backgroundColor;
	D2D1_SIZE_F _realSize; // real size of window
	D2D1_SIZE_F _camSize; // camera size (according to the screen size and _windowScale)
	float _windowScale;
	// we save `_realSize`, `_camSize`, and `_windowScale` to save time instead of calculating them every time
};
