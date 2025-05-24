#include "GameMainApp.h"
#include "Assets-Managers/AssetsManager.h"
#include "Menu/OpeningScreenEngine.h"

#ifdef _DEBUG
#include "Menu/LevelLoadingEngine.h"
#include "Menu/MenuEngine.h"
#endif
#include "SavedDataManager.h"


GameMainApp::GameMainApp()
	: BaseApp(WndProc, L"Claw")
{
	SavedDataManager::Initialize();
	AssetsManager::Initialize();
}
GameMainApp::~GameMainApp()
{
	AssetsManager::Finalize();
	SavedDataManager::Finalize();
}

void GameMainApp::init()
{
#ifdef _DEBUG
//	_pEngine = make_shared<MenuEngine>();
	_pEngine = make_shared<LevelLoadingEngine>(3);
#else
	_pEngine = make_shared<OpeningScreenEngine>();
#endif
}

LRESULT CALLBACK GameMainApp::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_SETCURSOR: SetCursor(NULL); return TRUE;
	default: break;
	}
	return BaseApp::WndProc(hwnd, message, wParam, lParam);
}
