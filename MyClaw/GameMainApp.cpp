#include "GameMainApp.h"
#include "Assets-Managers/AssetsManager.h"
#include "Menu/OpeningScreenEngine.h"

#ifdef _DEBUG
#include "Menu/LevelLoadingEngine.h"
#include "Menu/MenuEngine.h"
#endif 


GameMainApp::GameMainApp()
	: BaseApp(WndProc, L"Claw")
{
	AssetsManager::Initialize();
}
GameMainApp::~GameMainApp()
{
	AssetsManager::Finalize();
}

void GameMainApp::init()
{
#ifdef _DEBUG
	//_pEngine =  make_shared<OpeningScreenEngine>();
	//_pEngine =  make_shared<MenuEngine>();
	_pEngine =  make_shared<LevelLoadingEngine>(5);
#else
	_pEngine =  make_shared<OpeningScreenEngine>();
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
