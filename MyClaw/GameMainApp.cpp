#include "GameMainApp.h"
#include "Assets-Managers/AssetsManager.h"
#include "SavedDataManager.h"

#ifdef _DEBUG
#include "Menu/LevelLoadingEngine.h"
#include "Menu/MenuEngine.h"
#else
#include "Menu/OpeningScreenEngine.h"
#endif


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
//	_pEngine = DBG_NEW MenuEngine();
	_pEngine = DBG_NEW LevelLoadingEngine(1);
#else
	_pEngine = DBG_NEW OpeningScreenEngine();
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
