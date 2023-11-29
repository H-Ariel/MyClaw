#include "WindowManager.h"


static D2D1_POINT_2F defaultWindowOffset = {};

float WindowManager::PixelSize = 1; // min value: 1
HWND WindowManager::_hWnd = nullptr;
ID2D1Factory* WindowManager::_d2dFactory = nullptr;
IDWriteFactory* WindowManager::_dWriteFactory = nullptr;
ID2D1HwndRenderTarget* WindowManager::_renderTarget = nullptr;
IWICImagingFactory* WindowManager::_wicImagingFactory = nullptr;
const D2D1_POINT_2F* WindowManager::_windowOffset = &defaultWindowOffset;
ColorF WindowManager::_backgroundColor(0);
D2D1_SIZE_F WindowManager::realSize = DEFAULT_WINDOW_SIZE;

// throw exception if `func` failed
#define TRY_HRESULT(func, msg) if (FAILED(func)) throw Exception(msg);


void WindowManager::Initialize(const TCHAR WindowClassName[], void* lpParam)
{
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
void WindowManager::Finalize()
{
	SafeRelease(_renderTarget);
	SafeRelease(_d2dFactory);
	SafeRelease(_dWriteFactory);
	SafeRelease(_wicImagingFactory);
	CoUninitialize();
}

void WindowManager::setWindowOffset(const D2D1_POINT_2F* offset)
{
	if (offset)
		_windowOffset = offset;
	else
		_windowOffset = &defaultWindowOffset;
}

void WindowManager::resizeRenderTarget(D2D1_SIZE_U newSize)
{
	if (_renderTarget)
	{
		realSize = { (float)newSize.width, (float)newSize.height };
		_renderTarget->Resize(newSize);
	}
}

void WindowManager::drawRect(Rectangle2D dst, D2D1_COLOR_F color, float width)
{
	if (!_isInScreen(dst)) return;

	ID2D1SolidColorBrush* brush = nullptr;
	_renderTarget->CreateSolidColorBrush(color, &brush);
	if (brush)
	{
		dst.top *= PixelSize;
		dst.bottom *= PixelSize;
		dst.left *= PixelSize;
		dst.right *= PixelSize;

		_renderTarget->DrawRectangle(dst, brush, width);
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
	_renderTarget->CreateSolidColorBrush(color, &brush);
	if (brush)
	{
		dst.top *= PixelSize;
		dst.bottom *= PixelSize;
		dst.left *= PixelSize;
		dst.right *= PixelSize;

		_renderTarget->FillRectangle(dst, brush);
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
		_renderTarget->SetTransform(transformMatrix);
	}

	_renderTarget->DrawBitmap(bitmap, dst, opacity);

	if (mirrored)
	{
		// back to normal
		_renderTarget->SetTransform(Matrix3x2F::Identity());
	}
}
void WindowManager::drawText(const wstring& text, IDWriteTextFormat* textFormat, ID2D1SolidColorBrush* brush, const Rectangle2D& layoutRect)
{
	if (!textFormat || !brush) return;
	_renderTarget->DrawText(text.c_str(), (UINT32)text.length(), textFormat, layoutRect, brush);
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

	TRY_HRESULT(_wicImagingFactory->CreateBitmapFromMemory(
		width, height, GUID_WICPixelFormat32bppPRGBA,
		width * 4, width * height * 4, (BYTE*)buffer,
		&wicBitmap), "Failed to create WIC bitmap from buffer");

	TRY_HRESULT(_renderTarget->CreateBitmapFromWicBitmap(wicBitmap, &bitmap), "Failed to create D2D bitmap from WIC bitmap");

	SafeRelease(wicBitmap);

	return bitmap;
}
IDWriteTextFormat* WindowManager::createTextFormat(const FontData& font)
{
	IDWriteTextFormat* textFormat = nullptr;
	_dWriteFactory->CreateTextFormat(font.family.c_str(), nullptr, DWRITE_FONT_WEIGHT_NORMAL,
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
	_renderTarget->CreateSolidColorBrush(color, &brush);
	return brush;
}


bool WindowManager::isInScreen(Rectangle2D rc)
{
	return _isInScreen(rc);
}
bool WindowManager::_isInScreen(Rectangle2D& rc)
{
	rc.top -= _windowOffset->y;
	rc.bottom -= _windowOffset->y;
	rc.left -= _windowOffset->x;
	rc.right -= _windowOffset->x;

	const D2D1_SIZE_F wndSz = getSize();
	return (0 <= rc.right && rc.left < wndSz.width && 0 <= rc.bottom && rc.top < wndSz.height);
}
