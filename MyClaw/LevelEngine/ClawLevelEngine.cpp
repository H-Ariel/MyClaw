#include "ClawLevelEngineState.h"
#include "ActionPlane.h"
#include "CheatsManager.h"
#include "GlobalObjects.h"
#include "GameEngine/WindowManager.h"
#include "ScreenEngines/MenuEngine/MenuEngine.h"

constexpr auto& player = GO::player;


shared_ptr<ClawLevelEngine> ClawLevelEngine::_instance;


shared_ptr<ClawLevelEngine> ClawLevelEngine::create(int levelNumber, int checkpoint)
{
	_instance = shared_ptr<ClawLevelEngine>(DBG_NEW ClawLevelEngine(levelNumber, checkpoint));
	return _instance;
}

ClawLevelEngine::ClawLevelEngine(int levelNumber, int checkpoint)
	: actionPlane(nullptr), _saveBgColor(0), _saveWindowScale(1)
{
	_wwd = AssetsManager::loadLevel(levelNumber);

	for (WwdPlane& wwdPlane : _wwd->planes)
	{
		if (wwdPlane.flags & WwdPlane::WwdPlaneFlags_MainPlane)
		{
			actionPlane = DBG_NEW ActionPlane(_wwd.get(), &wwdPlane, this);
			_planes.push_back(shared_ptr<ActionPlane>(actionPlane));
		}
		else
		{
			_planes.push_back(make_shared<LevelPlane>(_wwd.get(), &wwdPlane));
		}
		_planes.back()->init();
	}

	if (!actionPlane) throw Exception("no main plane found"); // should never happen
	_hud = DBG_NEW LevelHUD(actionPlane);

	if (checkpoint != -1) // according to LevelLoadingEngine
		actionPlane->loadGame(levelNumber, checkpoint);

	WindowManager::setDefaultWindowScale();


	_state = DBG_NEW PlayState(this);
	_nextState = nullptr;

	for (shared_ptr<LevelPlane>& pln : _planes)
		_elementsList.push_back(pln.get());
	_elementsList.push_back(_hud);

	WindowManager::setWindowOffset(actionPlane->position);

	_inputController = DBG_NEW InputController(player.get());

#ifdef _DEBUG
	//if (levelNumber == 1) GO::getPlayerPosition() = { 3586, 4859 };
	//if (levelNumber == 1) GO::getPlayerPosition() = { 8537, 4430 };
	if (levelNumber == 1) GO::getPlayerPosition() = { 15511, 1514 };
	//if (levelNumber == 1) GO::getPlayerPosition() = { 17485, 1500 }; // END OF LEVEL
	//if (levelNumber == 1) GO::getPlayerPosition() = { 5775, 4347 };
	//if (levelNumber == 1) GO::getPlayerPosition() = { 9696, 772 };
	//if (levelNumber == 1) GO::getPlayerPosition() = { 5226, 4035 };
	//if (levelNumber == 1) GO::getPlayerPosition() = { 2596, 4155 };
	//if (levelNumber == 1) GO::getPlayerPosition() = { 4400, 4347 };
	//if (levelNumber == 1) GO::getPlayerPosition() = { 11039, 1851 };
	//if (levelNumber == 1) GO::getPlayerPosition() = { 2567, 4388 };
	//if (levelNumber == 1) GO::getPlayerPosition() = { 3123, 4219 };
	//if (levelNumber == 1) GO::getPlayerPosition() = { 9841, 4612 };

	//if (levelNumber == 2) GO::getPlayerPosition() = { 9196, 3958 };
	//if (levelNumber == 2) GO::getPlayerPosition() = { 16734, 1542 };
	//if (levelNumber == 2) GO::getPlayerPosition() = { 10881, 3382 };
	//if (levelNumber == 2) GO::getPlayerPosition() = { 593, 4086 };
	//if (levelNumber == 2) GO::getPlayerPosition() = { 17044, 3062 };
	//if (levelNumber == 2) GO::getPlayerPosition() = { 4596, 3958 };
	if (levelNumber == 2) GO::getPlayerPosition() = { 20070, 2092 }; // END OF LEVEL

	//if (levelNumber == 3) GO::getPlayerPosition() = { 23072, 6141 }; // ALMOST END OF LEVEL
	if (levelNumber == 3) GO::getPlayerPosition() = { 6080, 6224 };
	//if (levelNumber == 3) GO::getPlayerPosition() = { 2396, 1168 };
	//if (levelNumber == 3) GO::getPlayerPosition() = { 2201, 10756 };
	//if (levelNumber == 3) GO::getPlayerPosition() = { 9693, 8528 };
	//if (levelNumber == 3) GO::getPlayerPosition() = { 12289, 8144 };
	//if (levelNumber == 3) GO::getPlayerPosition() = { 11054, 8720 };

	//if (levelNumber == 4) GO::getPlayerPosition() = { 3127, 5024 };
	//if (levelNumber == 4) GO::getPlayerPosition() = { 3902, 4192 };
	//if (levelNumber == 4) GO::getPlayerPosition() = { 14500, 1464 }; // ALMOST END OF LEVEL
	if (levelNumber == 4) GO::getPlayerPosition() = { 16010, 1464 }; // END OF LEVEL
	//if (levelNumber == 4) GO::getPlayerPosition() = { 3902, 4192 };
	//if (levelNumber == 4) GO::getPlayerPosition() = { 3909, 4832 };

	if (levelNumber == 5) GO::getPlayerPosition() = { 9981, 3616 };
	if (levelNumber == 5) GO::getPlayerPosition() = { 10570, 3744 };
	//if (levelNumber == 5) GO::getPlayerPosition() = { 2620, 2848 };
	//if (levelNumber == 5) GO::getPlayerPosition() = { 7189, 3488 };
	//if (levelNumber == 5) GO::getPlayerPosition() = { 19041, 4511 };
	//if (levelNumber == 5) GO::getPlayerPosition() = { 6726, 2806 };

	//if (levelNumber == 6) GO::getPlayerPosition() = { 2676, 5878 };
	//if (levelNumber == 6) GO::getPlayerPosition() = { 2036, 5878 };
	if (levelNumber == 6) GO::getPlayerPosition() = { 30685, 4179 }; // END OF LEVEL

	//if (levelNumber == 7) GO::getPlayerPosition() = { 3428, 7254 };
	//if (levelNumber == 7) GO::getPlayerPosition() = { 10203, 7388 };
	//if (levelNumber == 7) GO::getPlayerPosition() = { 12316, 7388 };
	//if (levelNumber == 7) GO::getPlayerPosition() = { 15419, 7388 };
	if (levelNumber == 7) GO::getPlayerPosition() = { 25856, 6876 };
	//if (levelNumber == 7) GO::getPlayerPosition() = { 25210, 7196 };

	//if (levelNumber == 8) GO::getPlayerPosition() = { 4466, 5039 };
	//if (levelNumber == 8) GO::getPlayerPosition() = { 6160, 5232 };
	if (levelNumber == 8) GO::getPlayerPosition() = { 31535, 5267 }; // END OF LEVEL
	//if (levelNumber == 8) GO::getPlayerPosition() = { 9225, 5533 };
	//if (levelNumber == 8) GO::getPlayerPosition() = { 7153, 5725 };

	//if (levelNumber == 9) GO::getPlayerPosition() = { 5210, 7776 };
	//if (levelNumber == 9) GO::getPlayerPosition() = { 4220, 6484 };
	//if (levelNumber == 9) GO::getPlayerPosition() = { 4699, 6548 };
	//if (levelNumber == 9) GO::getPlayerPosition() = { 20928, 4057 };
	//if (levelNumber == 9) GO::getPlayerPosition() = { 8375, 6046};
	//if (levelNumber == 9) GO::getPlayerPosition() = { 7828, 7772 };
	//if (levelNumber == 9) GO::getPlayerPosition() = { 1609, 7252 };
	if (levelNumber == 9) GO::getPlayerPosition() = { 4800, 6500 };
	//if (levelNumber == 9) GO::getPlayerPosition() = { 4639, 5972 };
	//if (levelNumber == 9) GO::getPlayerPosition() = { 6756-32, 7640 };

	if (levelNumber == 10) GO::getPlayerPosition() = { 32368, 7769 }; // END OF LEVEL

	//if (levelNumber == 11) GO::getPlayerPosition() = { 5116, 1360 };
	//if (levelNumber == 11) GO::getPlayerPosition() = { 3616, 1486 };
	//if (levelNumber == 11) GO::getPlayerPosition() = { 13350, 1119 };
	//if (levelNumber == 11) GO::getPlayerPosition() = { 6837, 1360 };
	if (levelNumber == 11) GO::getPlayerPosition() = { 6067, 1452 };

	//if (levelNumber == 12) GO::getPlayerPosition() = { 7456, 2962 };
	//if (levelNumber == 12) GO::getPlayerPosition() = { 5986, 3091 };
	//if (levelNumber == 12) GO::getPlayerPosition() = { 11454, 3159 };
	//if (levelNumber == 12) GO::getPlayerPosition() = { 5839, 3608 };
	if (levelNumber == 12) GO::getPlayerPosition() = { 36324, 2072 }; // END OF LEVEL

	//if (levelNumber == 13) GO::getPlayerPosition() = { 18159, 2543 };
	if (levelNumber == 13) GO::getPlayerPosition() = { 32938, 2267 }; // END OF LEVEL
	//if (levelNumber == 13) GO::getPlayerPosition() = { 33002, 2267 }; // GOTO BOSS

	//if (levelNumber == 14) GO::getPlayerPosition() = { 22852, 2421 };
	//if (levelNumber == 14) GO::getPlayerPosition() = { 22477, 2244 };
	//if (levelNumber == 14) GO::getPlayerPosition() = { 3104, 3311 };
	//if (levelNumber == 14) GO::getPlayerPosition() = { 31043, 2317 }; // END OF LEVEL
	//if (levelNumber == 14) GO::getPlayerPosition() = { 41194, 1964 }; // in boss
	if (levelNumber == 14) GO::getPlayerPosition() = { 2858, 3371 };
	//if (levelNumber == 14) GO::getPlayerPosition() = { 8004, 2482 };
	//if (levelNumber == 14) GO::getPlayerPosition() = { 31137, 2348 }; // GOTO BOSS
	//if (levelNumber == 14) GO::getPlayerPosition() = { 7270, 4140 };
#endif
}
ClawLevelEngine::~ClawLevelEngine()
{
	delete _inputController;
	delete _state;
	delete _hud;
}

void ClawLevelEngine::playerEnterWrap(Warp* destinationWarp)
{
	switchState(DBG_NEW WrapCloseState(this, destinationWarp));
}
float ClawLevelEngine::getMaximalHoleRadius() const
{
	const D2D1_SIZE_F wndRealSz = WindowManager::getRealSize();
	return max(wndRealSz.width, wndRealSz.height) / 2;
}
void ClawLevelEngine::switchState(ClawLevelEngineState* newState)
{
	_nextState = newState; // maybe current state does not finished yet, so we wait for it
}

void ClawLevelEngine::Logic(uint32_t elapsedTime)
{
	_state->Logic(elapsedTime);

	if (_nextState)
	{
		delete _state;
		_state = _nextState;
		_nextState = nullptr;
	}
}
void ClawLevelEngine::Draw()
{
	BaseEngine::Draw();
	_state->Draw(); // special draw for state, e.g. wrap cover or death black screen
}

void ClawLevelEngine::OnKeyUp(int key)
{
	switch (key)
	{
	case VK_F1: // open help
		AssetsManager::playWavFile("STATES/MENU/SOUNDS/SELECT.WAV");
		_saveBgColor = WindowManager::getBackgroundColor();
		_saveWindowScale = WindowManager::getWindowScale();
		MenuEngine::setHelpScreen();
		changeEngine(make_shared<MenuEngine>());
		break;

	case VK_ESCAPE: // open pause menu
		AssetsManager::playWavFile("STATES/MENU/SOUNDS/SELECT.WAV");
		_saveBgColor = WindowManager::getBackgroundColor();
		_saveWindowScale = WindowManager::getWindowScale();
		MenuEngine::setIngameMenu();
		changeEngine(make_shared<MenuEngine>());
		break;

	default:
		_inputController->keyUp(key);
		break;
	}
}
void ClawLevelEngine::OnKeyDown(int key)
{
	_inputController->keyDown(key);
}
void ClawLevelEngine::OnResize()
{
	WindowManager::setDefaultWindowScale();
}
