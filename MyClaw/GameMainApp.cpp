#include "GameMainApp.h"
#include "Assets-Managers/AssetsManager.h"
#include "Menu/OpeningScreenEngine.h"
#include <chrono>

#ifdef _DEBUG
#include "Menu/LevelLoadingEngine.h"
#include "Menu/MenuEngine.h"
#include "Objects/Item.h"
#endif 


GameMainApp::GameMainApp()
{
	AssetsManager::Initialize();
}
GameMainApp::~GameMainApp()
{
	AssetsManager::Finalize();
}

void GameMainApp::run()
{
#ifdef _DEBUG
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
			Item::resetItemsPaths();
		}
	}
	else
	{
		runApp = true;
		//_pEngine = allocNewSharedPtr<OpeningScreenEngine>();
		//_pEngine = allocNewSharedPtr<MenuEngine>();
		_pEngine = allocNewSharedPtr<LevelLoadingEngine>(1);
		runEngine();
	}
#else
	runApp = true;
	_pEngine = allocNewSharedPtr<OpeningScreenEngine>();
	runEngine();
#endif
}
