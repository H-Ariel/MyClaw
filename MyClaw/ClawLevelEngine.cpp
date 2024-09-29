#include "ClawLevelEngine.h"
#include "ActionPlane.h"
#include "CheatsManager.h"
#include "GameEngine/WindowManager.h"
#include "Menu/HelpScreenEngine.h"
#include "Menu/LevelEndEngine.h"
#include "Menu/MenuEngine.h"


#define SCREEN_SPEED 0.5f // speed of the screen when CC is died or teleported
#define CC_FALLDEATH_SPEED 0.7f // speed of CC when he falls out the window

#define player BasePlaneObject::player


ClawLevelEngineFields::ClawLevelEngineFields(int levelNumber)
	: _mainPlanePosition(nullptr), _hud(nullptr), _saveBgColor(0), _saveWindowScale(1)
{
	_wwd = AssetsManager::loadLevel(levelNumber);

	for (WwdPlane& wwdPlane : _wwd->planes)
	{
		shared_ptr<LevelPlane> pln;
		if (wwdPlane.flags & WwdPlane::WwdPlaneFlags_MainPlane)
		{
			pln = make_shared<ActionPlane>(_wwd.get(), &wwdPlane);
			_mainPlanePosition = &pln->position;
		}
		else
		{
			pln = make_shared<LevelPlane>(_wwd.get(), &wwdPlane);
		}

		pln->init();
		_planes.push_back(pln);
	}

	if (!_mainPlanePosition) throw Exception("no main plane found"); // should never happen
	_hud = DBG_NEW LevelHUD(_mainPlanePosition);
}
ClawLevelEngineFields::~ClawLevelEngineFields()
{
	delete _hud;
}


ClawLevelEngine::ClawLevelEngine(int levelNumber, int checkpoint)
{
	if (checkpoint != -1) // according to LevelLoadingEngine
		ActionPlane::loadGame(levelNumber, checkpoint);
	_fields = make_shared<ClawLevelEngineFields>(levelNumber);
	WindowManager::setDefaultWindowScale();

	init();

#ifdef _DEBUG
	//	if (levelNumber == 1) BasePlaneObject::player->position = { 3586, 4859 };
	//	if (levelNumber == 1) BasePlaneObject::player->position = { 8537, 4430 };
	//	if (levelNumber == 1) BasePlaneObject::player->position = { 17485, 1500 }; // END OF LEVEL
	//	if (levelNumber == 1) BasePlaneObject::player->position = { 5775, 4347 };
	//	if (levelNumber == 1) BasePlaneObject::player->position = { 9696, 772 };
	//	if (levelNumber == 1) BasePlaneObject::player->position = { 5226, 4035 };
		if (levelNumber == 1) BasePlaneObject::player->position = { 2596, 4155 };
	//	if (levelNumber == 1) BasePlaneObject::player->position = { 4400, 4347 };
	//	if (levelNumber == 1) BasePlaneObject::player->position = { 11039, 1851 };
	//	if (levelNumber == 1) BasePlaneObject::player->position = { 2567, 4388 };
	//	if (levelNumber == 1) BasePlaneObject::player->position = { 3123, 4219 };
	//	if (levelNumber == 1) BasePlaneObject::player->position = { 9841, 4612 };

	//	if (levelNumber == 2) BasePlaneObject::player->position = { 9196, 3958 };
	//	if (levelNumber == 2) BasePlaneObject::player->position = { 16734, 1542 };
	//	if (levelNumber == 2) BasePlaneObject::player->position = { 10881, 3382 };
	//	if (levelNumber == 2) BasePlaneObject::player->position = { 593, 4086 };
	//	if (levelNumber == 2) BasePlaneObject::player->position = { 17044, 3062 };
	//	if (levelNumber == 2) BasePlaneObject::player->position = { 4596, 3958 };
	if (levelNumber == 2) BasePlaneObject::player->position = { 20070, 2092 }; // END OF LEVEL

	if (levelNumber == 3) BasePlaneObject::player->position = { 23072, 6141 }; // ALMOST END OF LEVEL
	//	if (levelNumber == 3) BasePlaneObject::player->position = { 6080, 6224 };
	//	if (levelNumber == 3) BasePlaneObject::player->position = { 2396, 1168 };
	//	if (levelNumber == 3) BasePlaneObject::player->position = { 2201, 10756 };
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
	if (levelNumber == 8) BasePlaneObject::player->position = { 31535, 5267 }; // END OF LEVEL
	//	if (levelNumber == 8) BasePlaneObject::player->position = { 9225, 5533 };
	//	if (levelNumber == 8) BasePlaneObject::player->position = { 7153, 5725 };

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
	if (levelNumber == 13) BasePlaneObject::player->position = { 32938, 2267 }; // END OF LEVEL
	//	if (levelNumber == 13) BasePlaneObject::player->position = { 33002, 2267 }; // GOTO BOSS

	//	if (levelNumber == 14) BasePlaneObject::player->position = { 22852, 2421 };
	//	if (levelNumber == 14) BasePlaneObject::player->position = { 22477, 2244 };
	//	if (levelNumber == 14) BasePlaneObject::player->position = { 3104, 3311 };
	//	if (levelNumber == 14) BasePlaneObject::player->position = { 31043, 2317 }; // END OF LEVEL
	//	if (levelNumber == 14) BasePlaneObject::player->position = { 41194, 1964 }; // in boss
	//	if (levelNumber == 14) BasePlaneObject::player->position = { 2858, 3371 };
	//	if (levelNumber == 14) BasePlaneObject::player->position = { 8004, 2482 };
	if (levelNumber == 14) BasePlaneObject::player->position = { 31137, 2348 }; // GOTO BOSS
	//	if (levelNumber == 14) BasePlaneObject::player->position = { 7270, 4140 };
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

	for (shared_ptr<LevelPlane>& pln : _fields->_planes)
		_elementsList.push_back(pln.get());
	_elementsList.push_back(_fields->_hud);

	WindowManager::setWindowOffset(*_fields->_mainPlanePosition);
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
			p->position = *_fields->_mainPlanePosition;

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
			else if (Warp::DestinationWarp)
			{
				_state = States::WrapClose;
				_wrapCoverTop = WindowManager::getCameraSize().height;
				_wrapDestination = Warp::DestinationWarp->getDestination();
				_isBossWarp = Warp::DestinationWarp->isBossWarp();
				_bossWarpX = Warp::DestinationWarp->position.x;

				if (Warp::DestinationWarp->removeObject)
					delete Warp::DestinationWarp; // ActionPlane didn't delete it for this case, so we need delete it here :)
				Warp::DestinationWarp = nullptr; // reset
			}
		}
		break;


	case States::DeathFall:
		player->position.y += CC_FALLDEATH_SPEED * elapsedTime;
		player->Logic(0); // update position of animation
		if (player->position.y - _fields->_mainPlanePosition->y > WindowManager::getCameraSize().height)
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
			ActionPlane::resetObjects();
			BaseEngine::Logic(elapsedTime);
			for (shared_ptr<LevelPlane>& p : _fields->_planes)
				p->position = *_fields->_mainPlanePosition;

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
			player->position = _wrapDestination;
			player->speed = {}; // stop player
			if (_isBossWarp)
			{
				ActionPlane::playerEnterToBoss(_bossWarpX);
				player->startPosition = _wrapDestination;
			}
			player->Logic(0); // update position of animation

			// update position of camera (because player position changed)
			ActionPlane::updatePosition();
			for (shared_ptr<LevelPlane>& p : _fields->_planes)
				p->position = *_fields->_mainPlanePosition;

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
			ScreenEngine::clearClawLevelEngineFields();
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
		WindowManager::drawHole(player->position, _holeRadius);
		break;

	case States::WrapClose:
	case States::WrapOpen:
		WindowManager::drawWrapCover(_wrapCoverTop);
		break;

	case States::GameOver:
		const D2D1_SIZE_F wndSz = WindowManager::getCameraSize();
		shared_ptr<UIBaseImage> img = AssetsManager::loadImage("GAME/IMAGES/MESSAGES/004.PID");
		img->position.x = _fields->_mainPlanePosition->x + wndSz.width / 2;
		img->position.y = _fields->_mainPlanePosition->y + wndSz.height / 2;
		img->Draw();
		break;
	}
}

void ClawLevelEngine::OnKeyUp(int key)
{
	if (key == 0xFF) return; // `Fn` key

	BasePlaneObject::cheats->addKey(key);

	switch (key)
	{
	case VK_F1: // open help
		AssetsManager::playWavFile("STATES/MENU/SOUNDS/SELECT.WAV");
		_fields->_saveBgColor = WindowManager::getBackgroundColor();
		_fields->_saveWindowScale = WindowManager::getWindowScale();
		changeEngine<HelpScreenEngine>(_fields);
		break;

	case VK_ESCAPE: // open pause menu
		AssetsManager::playWavFile("STATES/MENU/SOUNDS/SELECT.WAV");
		_fields->_saveBgColor = WindowManager::getBackgroundColor();
		_fields->_saveWindowScale = WindowManager::getWindowScale();
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
