#include "ClawLevelEngine.h"
#include "ActionPlane.h"
#include "CheatsManager.h"
#include "GlobalObjects.h"
#include "GameEngine/WindowManager.h"
#include "Menu/LevelEndEngine.h"
#include "Menu/MenuEngine.h"


#define SCREEN_SPEED 0.5f // speed of the screen when CC is died or teleported
#define CC_FALLDEATH_SPEED 0.7f // speed of CC when he falls out the window

#define player GO::player


ClawLevelEngineFields::ClawLevelEngineFields(int levelNumber, ClawLevelEngine* clawLevelEngine)
	: actionPlane(nullptr), _hud(nullptr), _saveBgColor(0), _saveWindowScale(1)
{
	_wwd = AssetsManager::loadLevel(levelNumber);

	for (WwdPlane& wwdPlane : _wwd->planes)
	{
		if (wwdPlane.flags & WwdPlane::WwdPlaneFlags_MainPlane)
		{
			actionPlane = DBG_NEW ActionPlane(_wwd.get(), &wwdPlane, clawLevelEngine);
			_planes.push_back(shared_ptr<ActionPlane>(actionPlane));
		}
		else
		{
			_planes.push_back(make_shared<LevelPlane>(_wwd.get(), &wwdPlane));
		}
	}

	//if (!actionPlane) throw Exception("no main plane found"); // should never happen
	_hud = DBG_NEW LevelHUD(actionPlane);
}
ClawLevelEngineFields::~ClawLevelEngineFields()
{
	delete _hud;
}


ClawLevelEngine::ClawLevelEngine(int levelNumber, int checkpoint)
{
	_fields = make_shared<ClawLevelEngineFields>(levelNumber, this);

	if (checkpoint != -1) // according to LevelLoadingEngine
		_fields->actionPlane->loadGame(levelNumber, checkpoint);

	WindowManager::setDefaultWindowScale();

	init();

#ifdef _DEBUG
	//	if (levelNumber == 1) GO::GO::getPlayerPosition() = { 3586, 4859 };
	//	if (levelNumber == 1) GO::GO::getPlayerPosition() = { 8537, 4430 };
	//	if (levelNumber == 1) GO::GO::getPlayerPosition() = { 17485, 1500 }; // END OF LEVEL
	//	if (levelNumber == 1) GO::GO::getPlayerPosition() = { 5775, 4347 };
	//	if (levelNumber == 1) GO::GO::getPlayerPosition() = { 9696, 772 };
	//	if (levelNumber == 1) GO::GO::getPlayerPosition() = { 5226, 4035 };
	//	if (levelNumber == 1) GO::GO::getPlayerPosition() = { 2596, 4155 };
		if (levelNumber == 1) GO::GO::getPlayerPosition() = { 4400, 4347 };
	//	if (levelNumber == 1) GO::GO::getPlayerPosition() = { 11039, 1851 };
	//	if (levelNumber == 1) GO::GO::getPlayerPosition() = { 2567, 4388 };
	//	if (levelNumber == 1) GO::GO::getPlayerPosition() = { 3123, 4219 };
	//	if (levelNumber == 1) GO::GO::getPlayerPosition() = { 9841, 4612 };

	//	if (levelNumber == 2) GO::GO::getPlayerPosition() = { 9196, 3958 };
	//	if (levelNumber == 2) GO::GO::getPlayerPosition() = { 16734, 1542 };
	//	if (levelNumber == 2) GO::GO::getPlayerPosition() = { 10881, 3382 };
	//	if (levelNumber == 2) GO::GO::getPlayerPosition() = { 593, 4086 };
	//	if (levelNumber == 2) GO::GO::getPlayerPosition() = { 17044, 3062 };
	//	if (levelNumber == 2) GO::GO::getPlayerPosition() = { 4596, 3958 };
	if (levelNumber == 2) GO::GO::getPlayerPosition() = { 20070, 2092 }; // END OF LEVEL

	//if (levelNumber == 3) GO::GO::getPlayerPosition() = { 23072, 6141 }; // ALMOST END OF LEVEL
	//	if (levelNumber == 3) GO::GO::getPlayerPosition() = { 6080, 6224 };
	//	if (levelNumber == 3) GO::GO::getPlayerPosition() = { 2396, 1168 };
	//	if (levelNumber == 3) GO::GO::getPlayerPosition() = { 2201, 10756 };
	//	if (levelNumber == 3) GO::GO::getPlayerPosition() = { 9693, 8528 };
	//	if (levelNumber == 3) GO::GO::getPlayerPosition() = { 12289, 8144 };
	//	if (levelNumber == 3) GO::GO::getPlayerPosition() = { 11054, 8720 };

	//	if (levelNumber == 4) GO::GO::getPlayerPosition() = { 3127, 5024 };
	//	if (levelNumber == 4) GO::GO::getPlayerPosition() = { 3902, 4192 };
	//	if (levelNumber == 4) GO::GO::getPlayerPosition() = { 14500, 1464 }; // ALMOST END OF LEVEL
	if (levelNumber == 4) GO::GO::getPlayerPosition() = { 16010, 1464 }; // END OF LEVEL
	//	if (levelNumber == 4) GO::GO::getPlayerPosition() = { 3902, 4192 };
	//	if (levelNumber == 4) GO::GO::getPlayerPosition() = { 3909, 4832 };

	if (levelNumber == 5) GO::GO::getPlayerPosition() = { 9981, 3616 };
	//	if (levelNumber == 5) GO::GO::getPlayerPosition() = { 2620, 2848 };
	//	if (levelNumber == 5) GO::GO::getPlayerPosition() = { 7189, 3488 };
	//	if (levelNumber == 5) GO::GO::getPlayerPosition() = { 19041, 4511 };
	//	if (levelNumber == 5) GO::GO::getPlayerPosition() = { 6726, 2806 };

	//	if (levelNumber == 6) GO::GO::getPlayerPosition() = { 2676, 5878 };
	//	if (levelNumber == 6) GO::GO::getPlayerPosition() = { 2036, 5878 };
	if (levelNumber == 6) GO::GO::getPlayerPosition() = { 30685, 4179 }; // END OF LEVEL

	//	if (levelNumber == 7) GO::GO::getPlayerPosition() = { 3428, 7254 };
	//	if (levelNumber == 7) GO::GO::getPlayerPosition() = { 10203, 7388 };
	//	if (levelNumber == 7) GO::GO::getPlayerPosition() = { 12316, 7388 };
	//	if (levelNumber == 7) GO::GO::getPlayerPosition() = { 15419, 7388 };
	if (levelNumber == 7) GO::GO::getPlayerPosition() = { 25856, 6876 };
	//	if (levelNumber == 7) GO::GO::getPlayerPosition() = { 25210, 7196 };

	//	if (levelNumber == 8) GO::GO::getPlayerPosition() = { 4466, 5039 };
	//	if (levelNumber == 8) GO::GO::getPlayerPosition() = { 6160, 5232 };
	if (levelNumber == 8) GO::GO::getPlayerPosition() = { 31535, 5267 }; // END OF LEVEL
	//	if (levelNumber == 8) GO::GO::getPlayerPosition() = { 9225, 5533 };
	//	if (levelNumber == 8) GO::GO::getPlayerPosition() = { 7153, 5725 };

	//	if (levelNumber == 9) GO::GO::getPlayerPosition() = { 5210, 7776 };
	//	if (levelNumber == 9) GO::GO::getPlayerPosition() = { 4220, 6484 };
	if (levelNumber == 9) GO::GO::getPlayerPosition() = { 4699, 6548 };
	//	if (levelNumber == 9) GO::GO::getPlayerPosition() = { 8375, 6046};
	//	if (levelNumber == 9) GO::GO::getPlayerPosition() = { 7828, 7772 };
	//	if (levelNumber == 9) GO::GO::getPlayerPosition() = { 1609, 7252 };
	//	if (levelNumber == 9) GO::GO::getPlayerPosition() = { 4800, 6590 };
	//	if (levelNumber == 9) GO::GO::getPlayerPosition() = { 4639, 5972 };
	//	if (levelNumber == 9) GO::GO::getPlayerPosition() = { 6756-32, 7640 };

	if (levelNumber == 10) GO::GO::getPlayerPosition() = { 32368, 7769 }; // END OF LEVEL

	//	if (levelNumber == 11) GO::GO::getPlayerPosition() = { 5116, 1360 };
	//	if (levelNumber == 11) GO::GO::getPlayerPosition() = { 3616, 1486 };
	//	if (levelNumber == 11) GO::GO::getPlayerPosition() = { 13350, 1119 };
	//	if (levelNumber == 11) GO::GO::getPlayerPosition() = { 6837, 1360 };
	if (levelNumber == 11) GO::GO::getPlayerPosition() = { 6067, 1452 };

	//	if (levelNumber == 12) GO::GO::getPlayerPosition() = { 7456, 2962 };
	//	if (levelNumber == 12) GO::GO::getPlayerPosition() = { 5986, 3091 };
	//	if (levelNumber == 12) GO::GO::getPlayerPosition() = { 11454, 3159 };
	//	if (levelNumber == 12) GO::GO::getPlayerPosition() = { 5839, 3608 };
	if (levelNumber == 12) GO::GO::getPlayerPosition() = { 36324, 2072 }; // END OF LEVEL

	//	if (levelNumber == 13) GO::GO::getPlayerPosition() = { 18159, 2543 };
	if (levelNumber == 13) GO::GO::getPlayerPosition() = { 32938, 2267 }; // END OF LEVEL
	//	if (levelNumber == 13) GO::GO::getPlayerPosition() = { 33002, 2267 }; // GOTO BOSS

	//	if (levelNumber == 14) GO::GO::getPlayerPosition() = { 22852, 2421 };
	//	if (levelNumber == 14) GO::GO::getPlayerPosition() = { 22477, 2244 };
	//	if (levelNumber == 14) GO::GO::getPlayerPosition() = { 3104, 3311 };
	//	if (levelNumber == 14) GO::GO::getPlayerPosition() = { 31043, 2317 }; // END OF LEVEL
	//	if (levelNumber == 14) GO::GO::getPlayerPosition() = { 41194, 1964 }; // in boss
		if (levelNumber == 14) GO::GO::getPlayerPosition() = { 2858, 3371 };
	//	if (levelNumber == 14) GO::GO::getPlayerPosition() = { 8004, 2482 };
	//	if (levelNumber == 14) GO::GO::getPlayerPosition() = { 31137, 2348 }; // GOTO BOSS
	//	if (levelNumber == 14) GO::GO::getPlayerPosition() = { 7270, 4140 };
#endif
}
ClawLevelEngine::ClawLevelEngine(shared_ptr<ClawLevelEngineFields> fields)
	: _fields(fields)
{
	WindowManager::setBackgroundColor(_fields->_saveBgColor);
	WindowManager::setWindowScale(_fields->_saveWindowScale);

	init();
}

void ClawLevelEngine::init()
{
	_holeRadius = 0;
	_state = States::Play;
	_wrapDestination = {};
	_wrapCoverTop = 0;
	_isBossWarp = false;
	_bossWarpX = 0;
	_gameOverTimeCounter = 0;

	for (shared_ptr<LevelPlane>& pln : _fields->_planes) {
		pln->init();
		_elementsList.push_back(pln.get());
	}
	_elementsList.push_back(_fields->_hud);

	WindowManager::setWindowOffset(_fields->actionPlane->position);
}

void ClawLevelEngine::Logic(uint32_t elapsedTime)
{
	const D2D1_SIZE_F wndRealSz = WindowManager::getRealSize();
	const float initialHoleRadius = max(wndRealSz.width, wndRealSz.height) / 2;

	switch (_state)
	{
	case States::Play:
		BaseEngine::Logic(elapsedTime);
		for (shared_ptr<LevelPlane>& p : _fields->_planes)
			p->position = _fields->actionPlane->position;

		_fields->_planes.back()->isVisible = SavedDataManager::instance.settings.frontLayer;

		if (player->isFinishDeathAnimation() && player->hasLives())
		{
			if (player->isSpikeDeath())
			{
				_holeRadius = initialHoleRadius;
				AssetsManager::playWavFile("GAME/SOUNDS/CIRCLEFADE.WAV");
				_state = States::DeathClose;
			}
			else //if (player->isFallDeath())
			{
				_state = States::DeathFall;
			}
		}
		else
		{
			if (player->isFinishLevel())
			{
				changeEngine<LevelEndEngine>(_fields->_wwd->levelNumber, player->getCollectedTreasures());
			}
			else if (!player->hasLives())
			{
				if (player->isFinishDeathAnimation())
				{
					_gameOverTimeCounter = 1500;
					_state = States::GameOver;
				}
			}
		}
		break;


	case States::DeathFall:
		GO::getPlayerPosition().y += CC_FALLDEATH_SPEED * elapsedTime;
		player->Logic(0); // update position of animation
		if (GO::getPlayerPosition().y - _fields->actionPlane->position.y > WindowManager::getCameraSize().height)
		{
			player->loseLife();
			_holeRadius = initialHoleRadius;
			AssetsManager::playWavFile("GAME/SOUNDS/CIRCLEFADE.WAV");
			_state = States::DeathClose;
		}
		break;

	case States::DeathClose:
		_holeRadius -= SCREEN_SPEED * elapsedTime;
		if (_holeRadius <= 0)
		{
			_holeRadius = 0;
			player->backToLife();

			// update position of camera (because player position changed)
			_fields->actionPlane->resetObjects();
			BaseEngine::Logic(elapsedTime);
			for (shared_ptr<LevelPlane>& p : _fields->_planes)
				p->position = _fields->actionPlane->position;

			AssetsManager::playWavFile("GAME/SOUNDS/FLAGWAVE.WAV");

			_state = States::DeathOpen;
		}
		break;

	case States::DeathOpen:
		_holeRadius += SCREEN_SPEED * elapsedTime;
		if (initialHoleRadius < _holeRadius)
			_state = States::Play;
		break;


	case States::WrapClose:
		_wrapCoverTop -= SCREEN_SPEED * elapsedTime;
		if (_wrapCoverTop <= 0)
		{
			GO::getPlayerPosition() = _wrapDestination;
			player->speed = {}; // stop player
			if (_isBossWarp)
			{
				_fields->actionPlane->playerEnterToBoss(_bossWarpX);
				player->startPosition = _wrapDestination;
			}
			player->Logic(0); // update position of animation

			// update position of camera (because player position changed)
			_fields->actionPlane->updatePosition();
			for (shared_ptr<LevelPlane>& p : _fields->_planes)
				p->position = _fields->actionPlane->position;

			_state = States::WrapOpen;
		}
		break;

	case States::WrapOpen:
		_wrapCoverTop -= SCREEN_SPEED * elapsedTime;
		if (_wrapCoverTop < -WindowManager::getCameraSize().height)
			_state = States::Play;
		break;


	case States::GameOver:
		_gameOverTimeCounter -= elapsedTime;
		if (_gameOverTimeCounter <= 0)
		{
			_gameOverTimeCounter = 0;
			MenuEngine::setMainMenu();
			changeEngine<MenuEngine>();
		}
		break;

	default: break;
	}
}
void ClawLevelEngine::Draw()
{
	BaseEngine::Draw();

	switch (_state)
	{
	case States::DeathClose:
	case States::DeathOpen:
		WindowManager::drawHole(GO::getPlayerPosition(), _holeRadius);
		break;

	case States::WrapClose:
	case States::WrapOpen:
		WindowManager::drawWrapCover(_wrapCoverTop);
		break;

	case States::GameOver:
		const D2D1_SIZE_F wndSz = WindowManager::getCameraSize();
		shared_ptr<UIBaseImage> img = AssetsManager::loadImage("GAME/IMAGES/MESSAGES/004.PID");
		img->position.x = _fields->actionPlane->position.x + wndSz.width / 2;
		img->position.y = _fields->actionPlane->position.y + wndSz.height / 2;
		img->Draw();
		break;
	}
}

void ClawLevelEngine::OnKeyUp(int key)
{
	GO::cheats->addKey(key);

	switch (key)
	{
	case VK_F1: // open help
		AssetsManager::playWavFile("STATES/MENU/SOUNDS/SELECT.WAV");
		_fields->_saveBgColor = WindowManager::getBackgroundColor();
		_fields->_saveWindowScale = WindowManager::getWindowScale();
		MenuEngine::setHelpScreen();
		changeEngine<MenuEngine>(_fields);
		break;

	case VK_ESCAPE: // open pause menu
		AssetsManager::playWavFile("STATES/MENU/SOUNDS/SELECT.WAV");
		_fields->_saveBgColor = WindowManager::getBackgroundColor();
		_fields->_saveWindowScale = WindowManager::getWindowScale();
		MenuEngine::setIngameMenu();
		changeEngine<MenuEngine>(_fields);
		break;

	default:
		player->keyUp(key);
		break;
	}
}
void ClawLevelEngine::OnKeyDown(int key)
{
	player->keyDown(key);
}

void ClawLevelEngine::OnResize()
{
	WindowManager::setDefaultWindowScale();
}

void ClawLevelEngine::playerEnterWrap(Warp* destinationWarp)
{
	_state = States::WrapClose;
	_wrapCoverTop = WindowManager::getCameraSize().height;
	_wrapDestination = destinationWarp->getDestination();
	_isBossWarp = destinationWarp->isBossWarp();
	_bossWarpX = destinationWarp->position.x;
}
