#pragma once

#include "UITextElement.h"
#include "UIColorfulImage.h"

/// <summary>
/// Manages window creation, rendering, and UI elements in a Direct2D-based graphical environment.
/// </summary>
class WindowManager
{
public:
	/// <summary>
	/// Initializes the window manager with specified window class, title, and additional parameters.
	/// </summary>
	/// <param name="WindowClassName">The class name for the window.</param>
	/// <param name="title">The title of the window.</param>
	/// <param name="lpParam">Additional parameters for initialization (typically passed to window procedure).</param>
	static void Initialize(const TCHAR WindowClassName[], const TCHAR title[], void* lpParam);

	/// <summary>
	/// Finalizes and cleans up window manager resources.
	/// </summary>
	static void Finalize();

	/// <summary>
	/// Sets the window's title.
	/// </summary>
	/// <param name="title">The new title for the window.</param>
	static void setTitle(const string& title);

	/// <summary>
	/// Sets the window offset (shift in X and Y).
	/// </summary>
	/// <param name="offset">A 2D point specifying the offset for the window.</param>
	static void setWindowOffset(const D2D1_POINT_2F& offset) { myMemCpy(instance->_windowOffset, offset); }

	/// <summary>
	/// Sets the background color of the window.
	/// </summary>
	/// <param name="bgColor">The background color to apply to the window.</param>
	static void setBackgroundColor(ColorF bgColor) { instance->_backgroundColor = bgColor; }

	/// <summary>
	/// Gets the background color of the window.
	/// </summary>
	/// <returns>The current background color of the window.</returns>
	static ColorF getBackgroundColor() { return instance->_backgroundColor; }

	/// <summary>
	/// Gets the camera size for rendering.
	/// </summary>
	/// <returns>The camera size as a 2D size (width, height).</returns>
	static D2D1_SIZE_F getCameraSize() { return instance->_camSize; };

	/// <summary>
	/// Gets the actual size of the window.
	/// </summary>
	/// <returns>The real size of the window (width, height).</returns>
	static D2D1_SIZE_F getRealSize() { return instance->_realSize; }

	/// <summary>
	/// Gets the window handle (HWND).
	/// </summary>
	/// <returns>The window handle (HWND) of the window.</returns>
	static HWND getHwnd() { return instance->_hWnd; }

	/// <summary>
	/// Sets the window scale factor.
	/// </summary>
	/// <param name="scale">The scale factor to apply to the window.</param>
	static void setWindowScale(float scale);

	/// <summary>
	/// Gets the current window scale factor.
	/// </summary>
	/// <returns>The current window scale factor.</returns>
	static float getWindowScale() { return instance->_windowScale; }

	/// <summary>
	/// Resets the window scale to default (640x480 resolution).
	/// </summary>
	static void setDefaultWindowScale();

	/// <summary>
	/// Checks if a given rectangle is fully within the window.
	/// </summary>
	/// <param name="rc">The rectangle to check for visibility within the window.</param>
	/// <returns>True if the rectangle is within the window, false otherwise.</returns>
	static bool isInCameraBounds(const Rectangle2D& rc);

	/// <summary>
	/// Resizes the rendering target.
	/// </summary>
	/// <param name="newSize">The new size for the render target (width, height).</param>
	static void resizeRenderTarget(D2D1_SIZE_U newSize);

	/// <summary>
	/// Begins the drawing process for the window.
	/// </summary>
	static void BeginDraw();

	/// <summary>
	/// Ends the drawing process for the window.
	/// </summary>
	static void EndDraw();

	/// <summary>
	/// Clears the window and fills it with the background color.
	/// </summary>
	static void Clear();

	/// <summary>
	/// Draws a rectangle on the window.
	/// </summary>
	/// <param name="dst">The rectangle area to draw.</param>
	/// <param name="color">The color to use for drawing the rectangle.</param>
	/// <param name="width">The line width for the rectangle. Default is 1.</param>
	static void drawRect(const Rectangle2D& dst, ColorF color, float width = 1);

	/// <summary>
	/// Fills a rectangle on the window with the specified color.
	/// </summary>
	/// <param name="dst">The rectangle area to fill.</param>
	/// <param name="color">The color to fill the rectangle with.</param>
	static void fillRect(const Rectangle2D& dst, ColorF color);

	/// <summary>
	/// Draws a bitmap (image) on the window with the specified transformations.
	/// </summary>
	/// <param name="bitmap">The bitmap image to draw.</param>
	/// <param name="dst">The destination rectangle where the image will be drawn.</param>
	/// <param name="mirrored">Whether to mirror the image horizontally.</param>
	/// <param name="upsideDown">Whether to flip the image vertically.</param>
	/// <param name="opacity">The opacity level of the image (0.0 to 1.0).</param>
	static void drawBitmap(ID2D1Bitmap* bitmap, const Rectangle2D& dst, bool mirrored, bool upsideDown, float opacity);

	/// <summary>
	/// Draws an image on the window.
	/// </summary>
	/// <param name="image">A pointer to the image (UIBaseImage) to be drawn on the window.</param>
	static void drawImage(UIBaseImage* image);

	/// <summary>
	/// Draws text on the window using a specified text format.
	/// </summary>
	/// <param name="text">The text string to display.</param>
	/// <param name="textFormat">The text format (e.g., font, size, style) to apply.</param>
	/// <param name="color">The color to use for the text.</param>
	/// <param name="layoutRect">The area within which the text will be laid out.</param>
	static void drawText(const wstring& text, IDWriteTextFormat* textFormat, ColorF color, const Rectangle2D& layoutRect);

	/// <summary>
	/// Draws text on the window using custom font data.
	/// </summary>
	/// <param name="text">The text string to display.</param>
	/// <param name="font">The custom font data to use for the text.</param>
	/// <param name="color">The color to use for the text.</param>
	/// <param name="layoutRect">The area within which the text will be laid out.</param>
	static void drawText(const wstring& text, const FontData& font, ColorF color, const Rectangle2D& layoutRect);

	/// <summary>
	/// Draws a transparent circle (hole) on the window. The background color is black.
	/// </summary>
	/// <param name="center">The center point of the circle.</param>
	/// <param name="radius">The radius of the circle.</param>
	static void drawHole(D2D1_POINT_2F center, float radius);

	/// <summary>
	/// Covers the screen with a black rectangle, starting at a given vertical position.
	/// </summary>
	/// <param name="top">The Y-coordinate for the top of the black rectangle. The height is the window's height.</param>
	static void drawWrapCover(float top); // TODO: find better name

	/// <summary>
	/// Creates a solid color brush from the specified color.
	/// </summary>
	/// <param name="color">The color to create the brush with.</param>
	/// <returns>A pointer to the created solid color brush.</returns>
	static ID2D1SolidColorBrush* getBrush(const ColorF& color);

	/// <summary>
	/// Creates a text format from the provided font data.
	/// </summary>
	/// <param name="font">The font data (e.g., size, family, weight) for the text format.</param>
	/// <returns>A pointer to the created text format object.</returns>
	static IDWriteTextFormat* createTextFormat(const FontData& font);

	/// <summary>
	/// Creates an image from an RGBA buffer and stores it in the cache with a specified key.
	/// </summary>
	/// <param name="key">The key to associate with the image for later retrieval.</param>
	/// <param name="buffer">The RGBA buffer containing image data.</param>
	/// <param name="width">The width of the image.</param>
	/// <param name="height">The height of the image.</param>
	/// <param name="offsetX">The X offset of the image (useful for animations).</param>
	/// <param name="offsetY">The Y offset of the image (useful for animations).</param>
	/// <returns>A smart pointer to the created image.</returns>
	static shared_ptr<UIBaseImage> createImage(const string& key, const void* const buffer, uint32_t width, uint32_t height, float offsetX, float offsetY);
	
	static shared_ptr<UIColorfulImage> createColorfulImage(const string& key, const void* const buffer, uint32_t width, uint32_t height, float offsetX, float offsetY, const vector<ColorF>& colors);

	/// <summary>
	/// Retrieves an image from the cache using its key.
	/// </summary>
	/// <param name="key">The key of the image to retrieve.</param>
	/// <returns>A smart pointer to the image if found, otherwise nullptr.</returns>
	static shared_ptr<UIBaseImage> getImage(const string& key);

	/// <summary>
	/// Clears images from the cache that match a key filter.
	/// </summary>
	/// <param name="filter">A filter function to determine which keys to remove from the cache.</param>
	static void clearImages(function<bool(const string&)> filter);

	/// <summary>
	/// The default window size.
	/// </summary>
	static const D2D1_SIZE_F DEFAULT_WINDOW_SIZE;

private:
	static void drawRect(const Rectangle2D& dst, D2D1_COLOR_F color, float width);
	static void fillRect(const Rectangle2D& dst, D2D1_COLOR_F color);
	static ID2D1SolidColorBrush* getBrush(D2D1_COLOR_F color);
	static bool isVisibleOnScreen(const Rectangle2D& rc); // return if `rc` is in the window area
	static Rectangle2D removeOffsets(const Rectangle2D& rc); // subtracts the window-offset from it

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
