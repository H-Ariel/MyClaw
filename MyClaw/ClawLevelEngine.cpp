#include "ClawLevelEngine.h"
#include "Assets-Managers/AssetsManager.h"
#include "GUI/WindowManager.h"
#include "LevelPlane.h"
#include "Menus-Engines/HelpScreenEngine.h"
#include "Menus-Engines/LevelEndEngine.h"


ClawLevelEngineFields::ClawLevelEngineFields(int levelNumber)
	: _mainPlanePosition(nullptr), _hud(nullptr), _saveBgColor(0), _levelNumber(levelNumber), _savePixelSize(0)
{
}
ClawLevelEngineFields::~ClawLevelEngineFields()
{
	delete _hud;
	AssetsManager::clearLevelAssets(_levelNumber);
}


ClawLevelEngine::ClawLevelEngine(int levelNumber)
{
	_fields = allocNewSharedPtr<ClawLevelEngineFields>(levelNumber);

	_fields->_wwd = AssetsManager::loadLevelWwdFile(levelNumber);
	for (shared_ptr<LevelPlane>& pln : _fields->_wwd->planes)
	{
		if (pln->isMainPlane())
			_fields->_mainPlanePosition = &pln->position;
		_elementsList.push_back(pln.get());
	}

	if (_fields->_mainPlanePosition == nullptr) throw Exception("no main plane found");

	_elementsList.push_back(_fields->_hud = DBG_NEW LevelHUD(*_fields->_mainPlanePosition));
	WindowManager::setWindowOffset(_fields->_mainPlanePosition);

#ifdef _DEBUG
//	if (levelNumber == 1) BasePlaneObject::player->position = { 3586, 4859 };
	if (levelNumber == 1) BasePlaneObject::player->position = { 8537, 4430};
//	if (levelNumber == 1) BasePlaneObject::player->position = { 17485, 1500 }; // END OF LEVEL
//	if (levelNumber == 1) BasePlaneObject::player->position = { 5775, 4347 };
//	if (levelNumber == 1) BasePlaneObject::player->position = { 9696, 772 };
//	if (levelNumber == 1) BasePlaneObject::player->position = { 5226, 4035 };
//	if (levelNumber == 1) BasePlaneObject::player->position = { 2596, 4155 };
//	if (levelNumber == 1) BasePlaneObject::player->position = { 4400, 4347 };
//	if (levelNumber == 1) BasePlaneObject::player->position = { 11039, 1851 };
//	if (levelNumber == 1) BasePlaneObject::player->position = { 2567, 4388 };
//	if (levelNumber == 1) BasePlaneObject::player->position = { 3123, 4219 };

//	if (levelNumber == 2) BasePlaneObject::player->position = { 9196, 3958 };
//	if (levelNumber == 2) BasePlaneObject::player->position = { 16734, 1542 };
//	if (levelNumber == 2) BasePlaneObject::player->position = { 10881, 3382 };
//	if (levelNumber == 2) BasePlaneObject::player->position = { 593, 4086 };
//	if (levelNumber == 2) BasePlaneObject::player->position = { 17044, 3062 };
//	if (levelNumber == 2) BasePlaneObject::player->position = { 4596, 3958 };
	if (levelNumber == 2) BasePlaneObject::player->position = { 20070, 2092 }; // END OF LEVEL

//	if (levelNumber == 3) BasePlaneObject::player->position = { 23072, 6141 }; // ALMOST END OF LEVEL
//	if (levelNumber == 3) BasePlaneObject::player->position = { 6080, 6224 };
//	if (levelNumber == 3) BasePlaneObject::player->position = { 2396, 1168 };
	if (levelNumber == 3) BasePlaneObject::player->position = { 2201, 10756 };
//	if (levelNumber == 3) BasePlaneObject::player->position = { 9693, 8528 };
//	if (levelNumber == 3) BasePlaneObject::player->position = { 12289, 8144 };
//	if (levelNumber == 3) BasePlaneObject::player->position = { 11054, 8720 };

//	if (levelNumber == 4) BasePlaneObject::player->position = { 3127, 5024 };
//	if (levelNumber == 4) BasePlaneObject::player->position = { 3902, 4192 };
//	if (levelNumber == 4) BasePlaneObject::player->position = { 14500, 1464 }; // ALMOST END OF LEVEL
	if (levelNumber == 4) BasePlaneObject::player->position = { 16010, 1464 }; // END OF LEVEL
//	if (levelNumber == 4) BasePlaneObject::player->position = { 3902, 4192 };
//	if (levelNumber == 4) BasePlaneObject::player->position = { 3909, 4832 };

	if (levelNumber == 5) BasePlaneObject::player->position = { 9981, 3616 };
//	if (levelNumber == 5) BasePlaneObject::player->position = { 2620, 2848 };
//	if (levelNumber == 5) BasePlaneObject::player->position = { 7189, 3488 };
//	if (levelNumber == 5) BasePlaneObject::player->position = { 19041, 4511 };
//	if (levelNumber == 5) BasePlaneObject::player->position = { 6726, 2806 };

//	if (levelNumber == 6) BasePlaneObject::player->position = { 2676, 5878 };
//	if (levelNumber == 6) BasePlaneObject::player->position = { 2036, 5878 };
	if (levelNumber == 6) BasePlaneObject::player->position = { 30685, 4179 }; // END OF LEVEL

//	if (levelNumber == 7) BasePlaneObject::player->position = { 3428, 7254 };
//	if (levelNumber == 7) BasePlaneObject::player->position = { 10203, 7388 };
//	if (levelNumber == 7) BasePlaneObject::player->position = { 12316, 7388 };
//	if (levelNumber == 7) BasePlaneObject::player->position = { 15419, 7388 };
	if (levelNumber == 7) BasePlaneObject::player->position = { 25856, 6876 };
//	if (levelNumber == 7) BasePlaneObject::player->position = { 25210, 7196 };

//	if (levelNumber == 8) BasePlaneObject::player->position = { 4466, 5039 };
//	if (levelNumber == 8) BasePlaneObject::player->position = { 6160, 5232 };
//	if (levelNumber == 8) BasePlaneObject::player->position = { 31535, 5267 }; // END OF LEVEL
//	if (levelNumber == 8) BasePlaneObject::player->position = { 9225, 5533 };
	if (levelNumber == 8) BasePlaneObject::player->position = { 7153, 5725 };

//	if (levelNumber == 9) BasePlaneObject::player->position = { 5210, 7776 };
//	if (levelNumber == 9) BasePlaneObject::player->position = { 4220, 6484 };
	if (levelNumber == 9) BasePlaneObject::player->position = { 4699, 6548 };
//	if (levelNumber == 9) BasePlaneObject::player->position = { 8375, 6046};
//	if (levelNumber == 9) BasePlaneObject::player->position = { 7828, 7772 };
//	if (levelNumber == 9) BasePlaneObject::player->position = { 1609, 7252 };
//	if (levelNumber == 9) BasePlaneObject::player->position = { 4800, 6590 };
//	if (levelNumber == 9) BasePlaneObject::player->position = { 4639, 5972 };
//	if (levelNumber == 9) BasePlaneObject::player->position = { 6756-32, 7640 };

	if (levelNumber == 10) BasePlaneObject::player->position = { 32368, 7769 }; // END OF LEVEL

//	if (levelNumber == 11) BasePlaneObject::player->position = { 5116, 1360 };
//	if (levelNumber == 11) BasePlaneObject::player->position = { 3616, 1486 };
//	if (levelNumber == 11) BasePlaneObject::player->position = { 13350, 1119 };
//	if (levelNumber == 11) BasePlaneObject::player->position = { 6837, 1360 };
	if (levelNumber == 11) BasePlaneObject::player->position = { 6067, 1452 };

//	if (levelNumber == 12) BasePlaneObject::player->position = { 7456, 2962 };
//	if (levelNumber == 12) BasePlaneObject::player->position = { 5986, 3091 };
//	if (levelNumber == 12) BasePlaneObject::player->position = { 11454, 3159 };
//	if (levelNumber == 12) BasePlaneObject::player->position = { 5839, 3608 };
	if (levelNumber == 12) BasePlaneObject::player->position = { 36324, 2072 }; // END OF LEVEL

//	if (levelNumber == 13) BasePlaneObject::player->position = { 18159, 2543 };
//	if (levelNumber == 13) BasePlaneObject::player->position = { 32938, 2267 };
//	if (levelNumber == 13) BasePlaneObject::player->position = { 33002, 2267 };

//	if (levelNumber == 14) BasePlaneObject::player->position = { 22852, 2421 };
//	if (levelNumber == 14) BasePlaneObject::player->position = { 22477, 2244 };
//	if (levelNumber == 14) BasePlaneObject::player->position = { 3104, 3311 };
//	if (levelNumber == 14) BasePlaneObject::player->position = { 31043, 2317 }; // END OF LEVEL
//	if (levelNumber == 14) BasePlaneObject::player->position = { 41194, 1964 }; // in boss
//	if (levelNumber == 14) BasePlaneObject::player->position = { 2858, 3371 };
	if (levelNumber == 14) BasePlaneObject::player->position = { 8004, 2482 };
#endif
}
ClawLevelEngine::ClawLevelEngine(shared_ptr<ClawLevelEngineFields> fields)
	: _fields(fields)
{
	for (shared_ptr<LevelPlane>& pln : _fields->_wwd->planes)
		_elementsList.push_back(pln.get());
	_elementsList.push_back(_fields->_hud);
	WindowManager::setWindowOffset(_fields->_mainPlanePosition);
	WindowManager::setBackgroundColor(_fields->_saveBgColor);
	WindowManager::PixelSize = _fields->_savePixelSize;
}

void ClawLevelEngine::Logic(uint32_t elapsedTime)
{
	BaseEngine::Logic(elapsedTime);

	for (shared_ptr<LevelPlane>& p : _fields->_wwd->planes)
		p->position = *_fields->_mainPlanePosition;

	if (!BasePlaneObject::player->hasLives())
	{
		if (BasePlaneObject::player->isFinishDeathAnimation())
		{
			MessageBoxA(nullptr, "You died", "", 0);
			// TODO: show GAME OVER screen and then go to menu
			changeEngine<MenuEngine>();
		}
	}
	else if (BasePlaneObject::player->isFinishLevel())
	{
		changeEngine<LevelEndEngine>(_fields->_levelNumber, BasePlaneObject::player->getCollectedTreasures());
	}
}

void ClawLevelEngine::OnKeyUp(int key)
{
	if (key == 0xFF) return; // `Fn` key

	if (key == VK_F1) // open help
	{
		_fields->_saveBgColor = WindowManager::getBackgroundColor();
		_fields->_savePixelSize = WindowManager::PixelSize;
		changeEngine<HelpScreenEngine>(_fields);
	}
	else if (key == VK_ESCAPE) // open pause menu
	{
		_fields->_saveBgColor = WindowManager::getBackgroundColor();
		_fields->_savePixelSize = WindowManager::PixelSize;
		changeEngine<MenuEngine>(_fields);
	}
	/*else if (key == VK_RETURN)
	{
		static int i = 0;
		i += 1;

		if (i == 3)
		{
			MessageBox(nullptr, L"now you are mega-player", L"cheat", 0);
			i = 0;
		}
	}*/
	else if (key == VK_ADD)
	{
		if (WindowManager::PixelSize <= 3.5f)
			WindowManager::PixelSize += 0.5f;
	}
	else if (key == VK_SUBTRACT)
	{
		if (WindowManager::PixelSize >= 1)
			WindowManager::PixelSize -= 0.5f;
	}
	else
	{
		BasePlaneObject::player->keyUp(key);
	}
}
void ClawLevelEngine::OnKeyDown(int key)
{
	BasePlaneObject::player->keyDown(key);
}
