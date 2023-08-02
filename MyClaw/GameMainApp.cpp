#include "GameMainApp.h"
#include "Assets-Managers/AssetsManager.h"
#include "WindowManager.h"
#include "ClawLevelEngine.h"
#include "MenuEngine.h"
#include <chrono>


#define WINDOW_CLASS_NAME L"MyGameWindow"

// The maximum iteration time.
// `elapsedTime` used in `EngineBase::Logic` and its overrides.
// safety so the game doesn't go wild.
#define MAX_ITER_TIME 20U


GameMainApp::GameMainApp()
	: _pEngine(nullptr)
{
	registerMyWindowClass();

	WindowManager::Initialize(WINDOW_CLASS_NAME, this);
	AssetsManager::Initialize();
}
GameMainApp::~GameMainApp()
{
	AssetsManager::Finalize();
	WindowManager::Finalize();
}

void GameMainApp::run()
{
	if (0) // TODO: delete this `if` block
	{
		// try load all levels
		for (int i = 1; i <= 14; i++)
		{
			try
			{
				cout << "load level " << i << endl;
				auto wwd = AssetsManager::loadLevelWwdFile(i);
				cout << endl;
			}
			catch (const Exception& e) {
				cout << " - failed: " << e.what() << endl;
			}
			AssetsManager::clearLevelAssets(i);
		}
	}
	else
	{
		runApp = true;
		//_pEngine = allocNewSharedPtr<MenuEngine>();
		_pEngine = allocNewSharedPtr<LevelLoadingEngine>(10);
		runEngine();
	}
}

void GameMainApp::runEngine()
{
	MSG msg;
	chrono::steady_clock::time_point begin, end;
	uint32_t elapsedTime; // in milliseconds

#ifdef _DEBUG
	// for FPS
	char fpsText[14] = {};
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
		_pEngine->Draw();

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

void GameMainApp::registerMyWindowClass()
{
	static bool doesClassRegistered = false;
	if (doesClassRegistered) return;
	WNDCLASSEXW wcex = {};
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = GameMainApp::WndProc;
	wcex.hInstance = HINST_THISCOMPONENT;
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszClassName = WINDOW_CLASS_NAME;
	RegisterClassEx(&wcex);
	doesClassRegistered = true;
}
LRESULT CALLBACK GameMainApp::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	shared_ptr<BaseEngine> engine;
	if (GameMainApp* app = (GameMainApp*)(GetWindowLongPtr(hwnd, GWLP_USERDATA)))
	{
		if (app->_pEngine)
		{
			engine = app->_pEngine;
		}
	}

	switch (message)
	{
	case WM_CREATE:			SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)(((LPCREATESTRUCT)lParam)->lpCreateParams)); break;
	case WM_GETMINMAXINFO:	((LPMINMAXINFO)lParam)->ptMinTrackSize = { 640,480 }; break;
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
