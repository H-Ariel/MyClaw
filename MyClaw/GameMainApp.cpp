#include "GameMainApp.h"
#include "Assets-Managers/AssetsManager.h"
#include "SavedDataManager.h"

#ifdef _DEBUG
#include "ScreenEngines/LevelLoadingEngine.h"
#include "ScreenEngines/MenuEngine/MenuEngine.h"
#else
#include "ScreenEngines/OpeningScreenEngine.h"
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
//	_pEngine = make_shared<MenuEngine>();
	_pEngine = make_shared<LevelLoadingEngine>(12);
#else
	_pEngine = make_shared<OpeningScreenEngine>();
#endif
}

LRESULT CALLBACK GameMainApp::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_SETCURSOR: SetCursor(NULL); return TRUE; // TODO set sword as cursor in menu
	default: break;
	}
	return BaseApp::WndProc(hwnd, message, wParam, lParam);
}
