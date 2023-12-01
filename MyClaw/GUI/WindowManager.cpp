#include "WindowManager.h"


// throw exception if `func` failed
#define TRY_HRESULT(func, msg) if (FAILED(func)) throw Exception(msg);

static const D2D1_POINT_2F defaultWindowOffset = {}; // empty point


float WindowManager::PixelSize = 1; // min value: 1 // TODO: rename to `WindowScale`
const D2D1_SIZE_F WindowManager::DEFAULT_WINDOW_SIZE = { 640.f, 480.f };
WindowManager* WindowManager::instance = nullptr;


WindowManager::WindowManager(const TCHAR WindowClassName[], void* lpParam)
	: _backgroundColor(0)
{
	_d2dFactory = nullptr;
	_dWriteFactory = nullptr;
	_renderTarget = nullptr;
	_wicImagingFactory = nullptr;
	_windowOffset = &defaultWindowOffset;
	realSize = DEFAULT_WINDOW_SIZE;

	_hWnd = CreateWindow(WindowClassName, L"", WS_OVERLAPPEDWINDOW, 100, 100, (int)realSize.width, (int)realSize.height, nullptr, nullptr, HINST_THISCOMPONENT, lpParam);
	if (!_hWnd) throw Exception("Failed to create window");
	ShowWindow(_hWnd, SW_SHOWDEFAULT);
	UpdateWindow(_hWnd);
	TRY_HRESULT(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &_d2dFactory), "Failed to create D2D1 factory");
	TRY_HRESULT(_d2dFactory->CreateHwndRenderTarget(RenderTargetProperties(), HwndRenderTargetProperties(_hWnd, { (UINT32)realSize.width, (UINT32)realSize.height }, D2D1_PRESENT_OPTIONS_NONE), &_renderTarget), "Failed to create render target");
	TRY_HRESULT(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(_dWriteFactory), (IUnknown**)(&_dWriteFactory)), "Faild to create write-factory");
	TRY_HRESULT(CoInitialize(nullptr), "Failed to initialize COM");
	TRY_HRESULT(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_IWICImagingFactory, (LPVOID*)(&_wicImagingFactory)), "Failed to create WIC imaging factory");

}
WindowManager::~WindowManager()
{
	SafeRelease(_renderTarget);
	SafeRelease(_d2dFactory);
	SafeRelease(_dWriteFactory);
	SafeRelease(_wicImagingFactory);
	CoUninitialize();
}

void WindowManager::Initialize(const TCHAR WindowClassName[], void* lpParam)
{
	if (instance == nullptr)
		instance = DBG_NEW WindowManager(WindowClassName, lpParam);
}
void WindowManager::Finalize()
{
	if (instance)
	{
		delete instance;
		instance = nullptr;
	}
}

void WindowManager::setTitle(const string& title) 
{
	SetWindowText(instance->_hWnd, wstring(title.begin(), title.end()).c_str());
}
void WindowManager::setWindowOffset(const D2D1_POINT_2F* offset)
{
	if (offset)
		instance->_windowOffset = offset;
	else
		instance->_windowOffset = &defaultWindowOffset;
}
void WindowManager::setBackgroundColor(ColorF bgColor) 
{
	instance->_backgroundColor = bgColor; 
}
ColorF WindowManager::getBackgroundColor()
{
	return instance->_backgroundColor; 
}
D2D1_SIZE_F WindowManager::getSize() 
{
	return { instance->realSize.width / PixelSize, instance->realSize.height / PixelSize };
}
D2D1_SIZE_F WindowManager::getRealSize() 
{ 
	return instance->realSize;
}
HWND WindowManager::getHwnd() 
{ 
	return instance->_hWnd;
}

bool WindowManager::isInScreen(Rectangle2D rc)
{
	return _isInScreen(rc);
}

void WindowManager::resizeRenderTarget(D2D1_SIZE_U newSize)
{
	if (instance && instance->_renderTarget)
	{
		instance->realSize = { (float)newSize.width, (float)newSize.height };
		instance->_renderTarget->Resize(newSize);
	}
}
void WindowManager::BeginDraw()
{
	instance->_renderTarget->BeginDraw();
	instance->_renderTarget->Clear(instance->_backgroundColor);
}
void WindowManager::EndDraw()
{
	instance->_renderTarget->EndDraw();
}
void WindowManager::drawRect(Rectangle2D dst, D2D1_COLOR_F color, float width)
{
	if (!_isInScreen(dst)) return;

	ID2D1SolidColorBrush* brush = nullptr;
	instance->_renderTarget->CreateSolidColorBrush(color, &brush);
	if (brush)
	{
		dst.top *= PixelSize;
		dst.bottom *= PixelSize;
		dst.left *= PixelSize;
		dst.right *= PixelSize;

		instance->_renderTarget->DrawRectangle(dst, brush, width);
		SafeRelease(brush);
	}
}
void WindowManager::drawRect(Rectangle2D dst, ColorF color, float width)
{
	drawRect(dst, (D2D1_COLOR_F)color, width);
}
void WindowManager::fillRect(Rectangle2D dst, D2D1_COLOR_F color)
{
	if (!_isInScreen(dst)) return;

	ID2D1SolidColorBrush* brush = nullptr;
	instance->_renderTarget->CreateSolidColorBrush(color, &brush);
	if (brush)
	{
		dst.top *= PixelSize;
		dst.bottom *= PixelSize;
		dst.left *= PixelSize;
		dst.right *= PixelSize;

		instance->_renderTarget->FillRectangle(dst, brush);
		SafeRelease(brush);
	}
}
void WindowManager::fillRect(Rectangle2D dst, ColorF color)
{
	fillRect(dst, (D2D1_COLOR_F)color);
}
void WindowManager::drawBitmap(ID2D1Bitmap* bitmap, Rectangle2D dst, bool mirrored, float opacity)
{
	if (!_isInScreen(dst) || bitmap == nullptr) return;

	dst.top *= PixelSize;
	dst.bottom *= PixelSize;
	dst.left *= PixelSize;
	dst.right *= PixelSize;

	if (mirrored)
	{
		// to draw mirrored we need to reverse the X-axis, so we change the sign of these points
		dst.left = -dst.left;
		dst.right = -dst.right;

		// set to draw mirrored
		D2D1_MATRIX_3X2_F transformMatrix = Matrix3x2F::Identity();
		transformMatrix._11 = -1;
		instance->_renderTarget->SetTransform(transformMatrix);
	}

	instance->_renderTarget->DrawBitmap(bitmap, dst, opacity);

	if (mirrored)
	{
		// back to normal
		instance->_renderTarget->SetTransform(Matrix3x2F::Identity());
	}
}
void WindowManager::drawText(const wstring& text, IDWriteTextFormat* textFormat, ID2D1SolidColorBrush* brush, const Rectangle2D& layoutRect)
{
	if (!textFormat || !brush) return;
	instance->_renderTarget->DrawText(text.c_str(), (UINT32)text.length(), textFormat, layoutRect, brush);
}
void WindowManager::drawText(const wstring& text, const FontData& font, const Rectangle2D& layoutRect, ColorF color)
{
	IDWriteTextFormat* textFormat = createTextFormat(font);
	ID2D1SolidColorBrush* brush = createSolidBrush(color);
	drawText(text, textFormat, brush, layoutRect);
	SafeRelease(brush);
	SafeRelease(textFormat);
}

ID2D1Bitmap* WindowManager::createBitmapFromBuffer(const void* const buffer, uint32_t width, uint32_t height)
{
	ID2D1Bitmap* bitmap = nullptr;
	IWICBitmap* wicBitmap = nullptr;

	TRY_HRESULT(instance->_wicImagingFactory->CreateBitmapFromMemory(
		width, height, GUID_WICPixelFormat32bppPRGBA,
		width * 4, width * height * 4, (BYTE*)buffer,
		&wicBitmap), "Failed to create WIC bitmap from buffer");

	TRY_HRESULT(instance->_renderTarget->CreateBitmapFromWicBitmap(wicBitmap, &bitmap), "Failed to create D2D bitmap from WIC bitmap");

	SafeRelease(wicBitmap);

	return bitmap;
}
IDWriteTextFormat* WindowManager::createTextFormat(const FontData& font)
{
	IDWriteTextFormat* textFormat = nullptr;
	instance->_dWriteFactory->CreateTextFormat(font.family.c_str(), nullptr, DWRITE_FONT_WEIGHT_NORMAL,
		DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, font.size, L"", &textFormat);

	if (textFormat)
	{
		switch (font.hAlignment)
		{
		case FontData::HorizontalAlignment::Right: textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING); break;
		case FontData::HorizontalAlignment::Center: textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER); break;
		case FontData::HorizontalAlignment::Left: textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING); break;
		}
		switch (font.vAlignment)
		{
		case FontData::VerticalAlignment::Top: textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR); break;
		case FontData::VerticalAlignment::Center: textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER); break;
		case FontData::VerticalAlignment::Bottom: textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_FAR); break;
		}
	}

	return textFormat;
}
ID2D1SolidColorBrush* WindowManager::createSolidBrush(ColorF color)
{
	ID2D1SolidColorBrush* brush = nullptr;
	instance->_renderTarget->CreateSolidColorBrush(color, &brush);
	return brush;
}

bool WindowManager::_isInScreen(Rectangle2D& rc)
{
	rc.top -= instance->_windowOffset->y;
	rc.bottom -= instance->_windowOffset->y;
	rc.left -= instance->_windowOffset->x;
	rc.right -= instance->_windowOffset->x;

	const D2D1_SIZE_F wndSz = getSize();
	return (0 <= rc.right && rc.left < wndSz.width && 0 <= rc.bottom && rc.top < wndSz.height);
}
