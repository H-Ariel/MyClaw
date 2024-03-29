#include "BaseApp.h"
#include "WindowManager.h"
#include <chrono>


#define WINDOW_CLASS_NAME L"MyGameWindow"

// The maximum iteration time.
// `elapsedTime` used in `EngineBase::Logic` and its overrides.
// safety so the game doesn't go wild.
#define MAX_ITER_TIME 20U


BaseApp::BaseApp()
	: _pEngine(nullptr)
{
	registerMyWindowClass();

	WindowManager::Initialize(WINDOW_CLASS_NAME, this);
}
BaseApp::~BaseApp()
{
	WindowManager::Finalize();
}

void BaseApp::run()
{
}

void BaseApp::runEngine()
{
	MSG msg;
	chrono::steady_clock::time_point begin, end;
	uint32_t elapsedTime; // in milliseconds

#ifdef _DEBUG
	// for FPS
	char fpsText[16] = {};
	uint32_t framesTime = 0, frames = 0;
#endif

	while (runApp && _pEngine && !_pEngine->StopEngine)
	{
		end = chrono::steady_clock::now();
		elapsedTime = (uint32_t)chrono::duration_cast<chrono::milliseconds>(end - begin).count();
		begin = end;

#ifdef _DEBUG
		framesTime += elapsedTime;
		frames++;
		if (framesTime > 1000)
		{
			sprintf(fpsText, "Game: %d FPS", frames);
			WindowManager::setTitle(fpsText);
			frames = 0;
			framesTime = 0;
		}
#endif

		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if (msg.message == WM_QUIT)
			{
				runApp = false;
				return;
			}
		}

		_pEngine->Logic(min(elapsedTime, MAX_ITER_TIME));
		WindowManager::BeginDraw();
		_pEngine->Draw();
		WindowManager::EndDraw();

		if (_pEngine->StopEngine)
		{
			_pEngine = _pEngine->getNextEngine();
			if (_pEngine == nullptr)
			{
				runApp = false;
				return;
			}
		}
	}
}

void BaseApp::registerMyWindowClass()
{
	static bool doesClassRegistered = false;
	if (doesClassRegistered) return;
	WNDCLASSEXW wcex = {};
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = BaseApp::WndProc;
	wcex.hInstance = HINST_THISCOMPONENT;
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszClassName = WINDOW_CLASS_NAME;
	RegisterClassEx(&wcex);
	doesClassRegistered = true;
}
LRESULT CALLBACK BaseApp::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	shared_ptr<BaseEngine> engine;
	if (BaseApp* app = (BaseApp*)(GetWindowLongPtr(hwnd, GWLP_USERDATA)))
	{
		if (app->_pEngine)
		{
			engine = app->_pEngine;
		}
	}

	switch (message)
	{
	case WM_CREATE:			SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)(((LPCREATESTRUCT)lParam)->lpCreateParams)); break;
	case WM_GETMINMAXINFO:	((LPMINMAXINFO)lParam)->ptMinTrackSize = {
			(LONG)WindowManager::DEFAULT_WINDOW_SIZE.width,
			(LONG)WindowManager::DEFAULT_WINDOW_SIZE.height
	}; break;
	case WM_DISPLAYCHANGE:	InvalidateRect(hwnd, nullptr, false); break;
	case WM_CLOSE:			PostQuitMessage(0); break;
	case WM_SETCURSOR:		SetCursor(NULL); return TRUE;
	case WM_MOUSEMOVE:		if (engine) engine->mousePosition = { LOWORD(lParam), HIWORD(lParam) }; break;
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:		if (engine) engine->OnKeyDown((int)wParam); break;
	case WM_KEYUP:
	case WM_SYSKEYUP:		if (engine) engine->OnKeyUp((int)wParam); break;
	case WM_LBUTTONUP:		if (engine) engine->OnMouseButtonUp(MouseButtons::Left); break;
	case WM_LBUTTONDOWN:	if (engine) engine->OnMouseButtonDown(MouseButtons::Left); break;
	case WM_MBUTTONUP:		if (engine) engine->OnMouseButtonUp(MouseButtons::Middle); break;
	case WM_MBUTTONDOWN:	if (engine) engine->OnMouseButtonDown(MouseButtons::Middle); break;
	case WM_RBUTTONUP:		if (engine) engine->OnMouseButtonUp(MouseButtons::Right); break;
	case WM_RBUTTONDOWN:	if (engine) engine->OnMouseButtonDown(MouseButtons::Right); break;
	case WM_SIZE: WindowManager::resizeRenderTarget(SizeU(LOWORD(lParam), HIWORD(lParam)));
		if (engine) engine->OnResize(); break;
	default: return DefWindowProc(hwnd, message, wParam, lParam);
	}

	return 0;
}
