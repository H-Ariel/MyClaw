#include "BaseApp.h"
#include "WindowManager.h"
#include "AudioManager.h"
#include <thread>
#include <chrono>


constexpr auto WINDOW_CLASS_NAME = L"ClawMainWindow";

// The maximum iteration time.
// `elapsedTime` used in `EngineBase::Logic` and its
// overrides. safety so the game doesn't go wild.
constexpr uint32_t MAX_ITER_TIME = 20U;

constexpr int targetFPS = 60;
constexpr DWORD frameDuration = 1000 / targetFPS;


BaseApp::BaseApp(WNDPROC wndproc, const TCHAR title[])
	: _pEngine(nullptr), _runApp(false)
{
	registerWindowClass(wndproc);

	WindowManager::Initialize(WINDOW_CLASS_NAME, title, this);
	AudioManager::Initialize();
}
BaseApp::~BaseApp()
{
	AudioManager::Finalize();
	WindowManager::Finalize();
}

void BaseApp::init()
{
	_pEngine = make_shared<BaseEngine>();
}
void BaseApp::run()
{
	timeBeginPeriod(1); // enable 1ms timer resolution for accurate frame timing

	MSG msg;
	DWORD frameEnd = timeGetTime(), lastFrameTime = timeGetTime();
	DWORD elapsed, workTime;
	uint32_t elapsedTime; // in milliseconds

#ifdef _DEBUG
	// for FPS
	char fpsText[16] = {};
	uint32_t framesTime = 0, frames = 0;
#endif

	_runApp = true;

	while (_runApp && _pEngine && !_pEngine->stopEngine)
	{
		elapsed = frameEnd - lastFrameTime;
		elapsedTime = min((uint32_t)elapsed, MAX_ITER_TIME);
		lastFrameTime = frameEnd;

#ifdef _DEBUG
		framesTime += elapsedTime;
		frames++;
		if (framesTime > 1000)
		{
			sprintf(fpsText, "%d FPS", frames);
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
				_runApp = false;
				return;
			}
		}

		_pEngine->Logic(elapsedTime);

		if (_pEngine->stopEngine)
		{
			_pEngine = _pEngine->getNextEngine();
			if (_pEngine == nullptr)
			{
				_runApp = false;
				return;
			}
			_pEngine->Logic(0); // just for update positions for drawing
		}

		WindowManager::BeginDraw();
		if (_pEngine->clearScreen)
			WindowManager::Clear();
		_pEngine->Draw();
		WindowManager::EndDraw();

		frameEnd = timeGetTime();
		workTime = frameEnd - lastFrameTime;
		if (workTime < frameDuration) // fit to 60 fps
			Sleep(frameDuration - workTime);
	}

	timeEndPeriod(1); // restore default timer resolution
}
LRESULT CALLBACK BaseApp::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	BaseEngine* engine = nullptr;
	if (BaseApp* app = (BaseApp*)(GetWindowLongPtr(hwnd, GWLP_USERDATA)))
	{
		if (app->_pEngine)
		{
			engine = app->_pEngine.get();
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

void BaseApp::registerWindowClass(WNDPROC wndproc)
{
	static bool doesClassRegistered = false;
	if (doesClassRegistered) return;
	WNDCLASSEXW wcex = {};
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = wndproc ? wndproc : BaseApp::WndProc;
	wcex.hInstance = HINST_THISCOMPONENT;
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = nullptr; // (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszClassName = WINDOW_CLASS_NAME;
	RegisterClassEx(&wcex);
	doesClassRegistered = true;
}
