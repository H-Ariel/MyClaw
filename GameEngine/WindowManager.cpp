﻿#include "WindowManager.h"
#include "AudioManager.h"

/*
TODO: save all D2D1 objects (IDWriteTextFormat) in a list and release them in `WindowManager::Finalize`
and create a function to get them with my format (like ID2D1SolidColorBrush and ID2D1Bitmap)
*/


bool operator<(ColorF a, ColorF b) { return memcmp(&a, &b, sizeof(ColorF)) < 0; }
bool operator==(ColorF a, ColorF b) { return memcmp(&a, &b, sizeof(ColorF)) == 0; }


const D2D1_SIZE_F WindowManager::DEFAULT_WINDOW_SIZE = { 640, 480 };
WindowManager* WindowManager::instance = nullptr;


WindowManager::WindowManager(const TCHAR WindowClassName[], const TCHAR title[], void* lpParam)
	: _windowOffset({}), _backgroundColor(0), _camSize(DEFAULT_WINDOW_SIZE), _realSize(DEFAULT_WINDOW_SIZE), _windowScale(1)
{
	_d2dFactory = nullptr;
	_dWriteFactory = nullptr;
	_renderTarget = nullptr;

	_hWnd = CreateWindow(WindowClassName, title, WS_OVERLAPPEDWINDOW, 100, 100, (int)_realSize.width, (int)_realSize.height, nullptr, nullptr, HINST_THISCOMPONENT, lpParam);
	if (!_hWnd) throw Exception("Failed to create window");
	ShowWindow(_hWnd, SW_SHOWDEFAULT);
	UpdateWindow(_hWnd);
	TRY_HRESULT(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &_d2dFactory), "Failed to create D2D1 factory");
	TRY_HRESULT(_d2dFactory->CreateHwndRenderTarget(RenderTargetProperties(), HwndRenderTargetProperties(_hWnd, { (UINT32)_realSize.width, (UINT32)_realSize.height }, D2D1_PRESENT_OPTIONS_NONE), &_renderTarget), "Failed to create render target");
	TRY_HRESULT(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(_dWriteFactory), (IUnknown**)(&_dWriteFactory)), "Faild to create write-factory");
}
WindowManager::~WindowManager()
{
	for (auto& i : images)
		i.second->_bitmap->Release();
	for (auto& i : brushes)
		i.second->Release();
	_renderTarget->Release();
	_d2dFactory->Release();
	_dWriteFactory->Release();
	DestroyWindow(_hWnd);
}

void WindowManager::Initialize(const TCHAR WindowClassName[], const TCHAR title[], void* lpParam)
{
	if (instance == nullptr)
		instance = DBG_NEW WindowManager(WindowClassName, title, lpParam);
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

void WindowManager::setWindowScale(float windowScale)
{
	if (windowScale > 3.5f) windowScale = 3.5f;
	else if (windowScale < 1) windowScale = 1;

	instance->_windowScale = windowScale;
	instance->_camSize = { instance->_realSize.width / windowScale, instance->_realSize.height / windowScale };

	instance->_renderTarget->SetTransform(Matrix3x2F::Scale(instance->_windowScale, instance->_windowScale));
}
void WindowManager::setDefaultWindowScale()
{
	setWindowScale(min(
		instance->_realSize.width / DEFAULT_WINDOW_SIZE.width,
		instance->_realSize.height / DEFAULT_WINDOW_SIZE.height
	));
}

bool WindowManager::isInScreen(Rectangle2D rc)
{
	return _isInScreen(rc);
}

void WindowManager::resizeRenderTarget(D2D1_SIZE_U newSize)
{
	if (instance && instance->_renderTarget)
	{
		instance->_realSize = { (float)newSize.width, (float)newSize.height };
		instance->_camSize = { newSize.width / instance->_windowScale, newSize.height / instance->_windowScale };
		instance->_renderTarget->Resize(newSize);
	}
}
void WindowManager::BeginDraw()
{
	instance->_renderTarget->BeginDraw();
}
void WindowManager::EndDraw()
{
	instance->_renderTarget->EndDraw();
}
void WindowManager::Clear()
{
	instance->_renderTarget->Clear(instance->_backgroundColor);
}

void WindowManager::drawRect(Rectangle2D dst, D2D1_COLOR_F color, float width)
{
	if (!_isInScreen(dst))
		return;
	if (ID2D1SolidColorBrush* brush = getBrush(color))
		instance->_renderTarget->DrawRectangle(dst, brush, width);
}
void WindowManager::drawRect(Rectangle2D dst, ColorF color, float width)
{
	drawRect(dst, (D2D1_COLOR_F)color, width);
}
void WindowManager::fillRect(Rectangle2D dst, D2D1_COLOR_F color)
{
	if (!_isInScreen(dst))
		return;
	if (ID2D1SolidColorBrush* brush = getBrush(color))
		instance->_renderTarget->FillRectangle(dst, brush);
}
void WindowManager::fillRect(Rectangle2D dst, ColorF color)
{
	fillRect(dst, (D2D1_COLOR_F)color);
}
void WindowManager::drawBitmap(ID2D1Bitmap* bitmap, Rectangle2D dst, bool mirrored, bool upsideDown, float opacity)
{
	// TODO find way to render more efficiently - maybe save last `transformMatrix` or render all `mirrored` and `upsideDown` at once

	if (!_isInScreen(dst) || bitmap == nullptr) return;

	Matrix3x2F originalTransform;
	instance->_renderTarget->GetTransform(&originalTransform);

	// apply transformations if mirrored or upside down
	if (mirrored || upsideDown) {
		Matrix3x2F transformMatrix = originalTransform;
		if (mirrored) {
			transformMatrix.dx = (dst.left + dst.right) * transformMatrix.m11; // set offset of X-axis to draw mirrored
			transformMatrix.m11 = -transformMatrix.m11; // set to draw mirrored (reverse the X-axis)
		}
		if (upsideDown) {
			transformMatrix.dy = (dst.top + dst.bottom) * transformMatrix.m22; // set offset of Y-axis to draw upside down
			transformMatrix.m22 = -transformMatrix.m22; // reverse the Y-axis for upside-down
		}
		instance->_renderTarget->SetTransform(transformMatrix);
	}

	// draw the bitmap
	instance->_renderTarget->DrawBitmap(bitmap, dst, opacity);

	// reset to the original transformation
	if (mirrored || upsideDown) {
		instance->_renderTarget->SetTransform(originalTransform);
	}
}
void WindowManager::drawImage(UIBaseImage* image)
{
	WindowManager::drawBitmap(image->_bitmap, image->GetRect(), image->mirrored, image->upsideDown, image->opacity);
}
void WindowManager::drawText(const wstring& text, IDWriteTextFormat* textFormat, ColorF color, const Rectangle2D& _layoutRect)
{
	ID2D1SolidColorBrush* brush = getBrush(color);
	if (!textFormat || !brush) return;

	// convert layoutRect to the window scale
	Rectangle2D layoutRect(_layoutRect);
	layoutRect.left /= instance->_windowScale;
	layoutRect.right /= instance->_windowScale;
	layoutRect.top /= instance->_windowScale;
	layoutRect.bottom /= instance->_windowScale;

	instance->_renderTarget->DrawText(text.c_str(), (UINT32)text.length(), textFormat, layoutRect, brush);
}
void WindowManager::drawText(const wstring& text, const FontData& font, ColorF color, const Rectangle2D& layoutRect)
{
	IDWriteTextFormat* textFormat = createTextFormat(font);
	drawText(text, textFormat, color, layoutRect);
	textFormat->Release();
}

void WindowManager::drawHole(D2D1_POINT_2F center, float radius)
{
	ID2D1EllipseGeometry* hole = nullptr;
	ID2D1RectangleGeometry* background = nullptr;
	ID2D1GeometryGroup* group = nullptr;
	ID2D1Geometry* groupItems[2] = {};
	ID2D1SolidColorBrush* brush = nullptr;

	center.x -= instance->_windowOffset.x;
	center.y -= instance->_windowOffset.y;
	instance->_d2dFactory->CreateEllipseGeometry(Ellipse(center, radius, radius), &hole);
	instance->_d2dFactory->CreateRectangleGeometry(RectF(instance->_realSize.width, instance->_realSize.height), &background);
	if (!hole || !background) goto end;

	groupItems[0] = background;
	groupItems[1] = hole;
	instance->_d2dFactory->CreateGeometryGroup(D2D1_FILL_MODE_ALTERNATE, groupItems, ARRAYSIZE(groupItems), &group);
	if (!group) goto end;

	brush = getBrush(ColorF::Black);
	if (!brush) goto end;

	instance->_renderTarget->FillGeometry(group, brush);

end:
	if (group) group->Release();
	if (background) background->Release();
	if (hole) hole->Release();
}
void WindowManager::drawWrapCover(float top)
{
	// for now, just draw a black rectangle... I want to make it better in the future
	if (ID2D1SolidColorBrush* brush = getBrush(ColorF::Black))
	{
		top *= instance->_windowScale;
		instance->_renderTarget->FillRectangle(RectF(0, top, instance->_realSize.width, top + instance->_realSize.height), brush);
	}
}

ID2D1SolidColorBrush* WindowManager::getBrush(D2D1_COLOR_F color)
{
	return getBrush(*((ColorF*)&color));
}
ID2D1SolidColorBrush* WindowManager::getBrush(ColorF color)
{
	if (instance->brushes.count(color) == 0)
	{
		ID2D1SolidColorBrush* brush = nullptr;
		instance->_renderTarget->CreateSolidColorBrush(color, &brush);

		if (brush)
			instance->brushes[color] = brush;
	}

	return instance->brushes[color];
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
shared_ptr<UIBaseImage> WindowManager::createImage(const string& key, const void* const buffer, uint32_t width, uint32_t height, float offsetX, float offsetY)
{
	if (instance->images.count(key) == 0)
	{
		ID2D1Bitmap* bitmap = nullptr;

		TRY_HRESULT(instance->_renderTarget->CreateBitmap(
			{ width, height }, buffer, width * 4,
			BitmapProperties(PixelFormat(DXGI_FORMAT_R8G8B8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)),
			&bitmap),
			"Failed to create D2D bitmap");

		instance->images[key] = make_shared<UIBaseImage>(bitmap, Point2F(offsetX, offsetY));
	}

	return instance->images[key];
}

static string colorToString(const ColorF& color) {
	char buf[32] = {};
	sprintf(buf, "%.2f %.2f %.2f %.2f", color.r, color.g, color.b, color.a);
	return buf;
}

shared_ptr<UIColorfulImage> WindowManager::createColorfulImage(const string& key, const void* const buffer, uint32_t width, uint32_t height, float offsetX, float offsetY, const vector<ColorF>& colors)
{
	// TODO better palce for colorCompare
	static function<bool(const ColorF&, const ColorF&)> colorCompare = [](const ColorF& x, const ColorF& y) {
		if (x.r != y.r) return x.r < y.r;
		if (x.g != y.g) return x.g < y.g;
		if (x.b != y.b) return x.b < y.b;
		return x.a < y.a;
	};

	shared_ptr<UIColorfulImage> colorfullyImage;

	if (instance->images.count(key) == 0)
	{
		map<ColorF, ID2D1Bitmap*, function<bool(const ColorF&, const ColorF&)>> images(colorCompare);

		ID2D1Bitmap* bitmap = nullptr;

		TRY_HRESULT(instance->_renderTarget->CreateBitmap(
			{ width, height }, buffer, width * 4,
			BitmapProperties(PixelFormat(DXGI_FORMAT_R8G8B8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)),
			&bitmap),
			"Failed to create D2D bitmap");

		images[ColorF(0, 0, 0, 0)] = bitmap; // regular image

		uint32_t len = width * height * 4;
		uint8_t* coloredBuffer = DBG_NEW uint8_t[len];
		memcpy(coloredBuffer, buffer, len);

		for (const ColorF& color : colors) {
			for (uint32_t i = 0; i < len; i += 4) {
				if (coloredBuffer[i + 3] != 0) {  // not transparent pixel
					coloredBuffer[i + 0] = (uint8_t)(color.r * 255);
					coloredBuffer[i + 1] = (uint8_t)(color.g * 255);
					coloredBuffer[i + 2] = (uint8_t)(color.b * 255);
				}
			}

			TRY_HRESULT(instance->_renderTarget->CreateBitmap(
				{ width, height }, coloredBuffer, width * 4,
				BitmapProperties(PixelFormat(DXGI_FORMAT_R8G8B8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)),
				&(images[color])),
				"Failed to create D2D bitmap " + colorToString(color));
		}
		delete[] coloredBuffer;
		colorfullyImage = make_shared<UIColorfulImage>(images, Point2F(offsetX, offsetY));
		instance->images[key] = colorfullyImage;
	}
 
 	return colorfullyImage;
}
shared_ptr<UIBaseImage> WindowManager::getImage(const string& key)
{
	auto it = instance->images.find(key);
	if (it == instance->images.end())
		return nullptr;
	return it->second;
}
void WindowManager::clearImages(function<bool(const string&)> filter)
{
	for (auto it = instance->images.begin(); it != instance->images.end();)
	{
		if (filter(it->first))
			it = instance->images.erase(it);
		else
			++it;
	}
}

// remove the window-offset from `rc` and return if it's in the window area
bool WindowManager::_isInScreen(Rectangle2D& rc)
{
	rc.top -= instance->_windowOffset.y;
	rc.bottom -= instance->_windowOffset.y;
	rc.left -= instance->_windowOffset.x;
	rc.right -= instance->_windowOffset.x;

	return (0 <= rc.right && rc.left < instance->_camSize.width && 0 <= rc.bottom && rc.top < instance->_camSize.height);
}
