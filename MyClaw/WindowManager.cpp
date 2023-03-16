#include "WindowManager.h"


static D2D1_POINT_2F defaultWindowOffset = {};

float WindowManager::PixelSize = 1; // min value: 1
HWND WindowManager::_hWnd = nullptr;
ID2D1Factory* WindowManager::_d2dFactory = nullptr;
ID2D1HwndRenderTarget* WindowManager::_renderTarget = nullptr;
IWICImagingFactory* WindowManager::_wicImagingFactory = nullptr;
const D2D1_POINT_2F* WindowManager::_windowOffset = &defaultWindowOffset;
ColorF WindowManager::_backgroundColor(0);


void WindowManager::Initialize(const TCHAR WindowClassName[], void* lpParam)
{
	_hWnd = CreateWindow(WindowClassName, L"", WS_OVERLAPPEDWINDOW, 100, 100, 800, 600, nullptr, nullptr, HINST_THISCOMPONENT, lpParam);
	WINAPI_THROW_IF_NULL(_hWnd);
	ShowWindow(_hWnd, SW_SHOWDEFAULT);
	UpdateWindow(_hWnd);
	D2D1_SIZE_F wndSz = getRealSize();
	HRESULT_THROW_IF_FAILED(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &_d2dFactory));
	HRESULT_THROW_IF_FAILED(_d2dFactory->CreateHwndRenderTarget(RenderTargetProperties(), HwndRenderTargetProperties(_hWnd, { (UINT32)wndSz.width, (UINT32)wndSz.height }, D2D1_PRESENT_OPTIONS_NONE), &_renderTarget));
	HRESULT_THROW_IF_FAILED(CoInitialize(nullptr));
	HRESULT_THROW_IF_FAILED(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_IWICImagingFactory, (LPVOID*)(&_wicImagingFactory)));
}
void WindowManager::Finalize()
{
	SafeRelease(&_renderTarget);
	SafeRelease(&_d2dFactory);
	SafeRelease(&_wicImagingFactory);
	CoUninitialize();
};

void WindowManager::setSize(D2D1_SIZE_F size)
{
	RECT rc = {};
	GetWindowRect(_hWnd, &rc);
	SetWindowPos(_hWnd, NULL, rc.left, rc.top, (int)size.width, (int)size.height, SWP_NOMOVE);
}

void WindowManager::setWindowOffset(const D2D1_POINT_2F* offset)
{
	if (offset)
		_windowOffset = offset;
	else
		_windowOffset = &defaultWindowOffset;
}

D2D1_SIZE_F WindowManager::getSize()
{
	D2D1_SIZE_F size = getRealSize();
	return { size.width / PixelSize, size.height / PixelSize };
}
D2D1_SIZE_F WindowManager::getRealSize()
{
	RECT rc = {};
	GetWindowRect(_hWnd, &rc);
	return SizeF((float)(rc.right - rc.left), (float)(rc.bottom - rc.top));
}

void WindowManager::resizeRenderTarget(D2D1_SIZE_U newSize)
{
	if (_renderTarget)
	{
		_renderTarget->Resize(newSize);
	}
}

void WindowManager::drawRect(D2D1_RECT_F dst, D2D1_COLOR_F color, float width)
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
		SafeRelease(&brush);
	}
}
void WindowManager::drawRect(D2D1_RECT_F dst, ColorF color, float width)
{
	drawRect(dst, (D2D1_COLOR_F)color, width);
}
void WindowManager::fillRect(D2D1_RECT_F dst, D2D1_COLOR_F color)
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
		SafeRelease(&brush);
	}
}
void WindowManager::fillRect(D2D1_RECT_F dst, ColorF color)
{
	fillRect(dst, (D2D1_COLOR_F)color);
}
void WindowManager::drawCircle(D2D1_POINT_2F center, FLOAT radius, ColorF color, float width)
{
	D2D1_RECT_F dst = { center.x - radius, center.y - radius, center.x + radius, center.y + radius };
	if (!_isInScreen(dst)) return;

	ID2D1SolidColorBrush* brush = nullptr;
	_renderTarget->CreateSolidColorBrush(color, &brush);
	if (brush)
	{
		D2D1_ELLIPSE el = { { center.x * PixelSize, center.y * PixelSize }, radius * PixelSize, radius * PixelSize };
		_renderTarget->DrawEllipse(el, brush, width);
		SafeRelease(&brush);
	}
}
void WindowManager::drawBitmap(ID2D1Bitmap* bitmap, D2D1_RECT_F dst, bool mirrored)
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

	_renderTarget->DrawBitmap(bitmap, dst);

	if (mirrored)
	{
		// back to normal
		_renderTarget->SetTransform(Matrix3x2F::Identity());
	}
}

ID2D1Bitmap* WindowManager::createBitmapFromBuffer(const void* const buffer, uint32_t width, uint32_t height)
{
	ID2D1Bitmap* bitmap = nullptr;
	IWICBitmap* wicBitmap = nullptr;

	HRESULT_THROW_IF_FAILED(_wicImagingFactory->CreateBitmapFromMemory(
		width, height,
		GUID_WICPixelFormat32bppPRGBA,
		width * 4,
		width * height * 4,
		(BYTE*)buffer,
		&wicBitmap));

	HRESULT_THROW_IF_FAILED(_renderTarget->CreateBitmapFromWicBitmap(wicBitmap, &bitmap));

	SafeRelease(&wicBitmap);

	return bitmap;
}

bool WindowManager::isInScreen(D2D1_RECT_F rc)
{
	return _isInScreen(rc);
}
bool WindowManager::_isInScreen(D2D1_RECT_F& rc)
{
	rc.top -= _windowOffset->y;
	rc.bottom -= _windowOffset->y;
	rc.left -= _windowOffset->x;
	rc.right -= _windowOffset->x;

	const D2D1_SIZE_F wndSz = getSize();
	return (0 <= rc.right && rc.left < wndSz.width && 0 <= rc.bottom && rc.top < wndSz.height);
}
