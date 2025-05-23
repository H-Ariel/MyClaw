#include "ClawLevelEngine.h"
#include "ActionPlane.h"
#include "CheatsManager.h"
#include "GlobalObjects.h"
#include "GameEngine/WindowManager.h"
#include "Menu/LevelEndEngine.h"
#include "Menu/MenuEngine.h"


constexpr float SCREEN_SPEED = 0.5f; // speed of the screen when CC is died or teleported
constexpr float CC_FALLDEATH_SPEED = 0.7f; // speed of CC when he falls out the window

#define player GO::player

class PlayState : public ClawLevelEngineState {
public:
	PlayState(ClawLevelEngine* clawLevelEngine)
		: ClawLevelEngineState(StateType::Play, clawLevelEngine) {}
	void Logic(uint32_t elapsedTime) override;
};
class DeathFallState : public ClawLevelEngineState {
public:
	DeathFallState(ClawLevelEngine* clawLevelEngine)
		: ClawLevelEngineState(StateType::DeathFall, clawLevelEngine) {}
	void Logic(uint32_t elapsedTime) override;
};
class DeathCloseState : public ClawLevelEngineState {
public:
	DeathCloseState(ClawLevelEngine* clawLevelEngine)
		: ClawLevelEngineState(StateType::DeathClose, clawLevelEngine) {}
	void Logic(uint32_t elapsedTime) override;
};
class DeathOpenState : public ClawLevelEngineState {
public:
	DeathOpenState(ClawLevelEngine* clawLevelEngine)
		: ClawLevelEngineState(StateType::DeathOpen, clawLevelEngine) {}
	void Logic(uint32_t elapsedTime) override;
};
class WrapCloseState : public ClawLevelEngineState {
public:
	WrapCloseState(ClawLevelEngine* clawLevelEngine)
		: ClawLevelEngineState(StateType::WrapClose, clawLevelEngine) {}
	void Logic(uint32_t elapsedTime) override;
};
class WrapOpenState : public ClawLevelEngineState {
public:
	WrapOpenState(ClawLevelEngine* clawLevelEngine)
		: ClawLevelEngineState(StateType::WrapOpen, clawLevelEngine) {}
	void Logic(uint32_t elapsedTime) override;
};
class GameOverState : public ClawLevelEngineState {
public:
	GameOverState(ClawLevelEngine* clawLevelEngine)
		: ClawLevelEngineState(StateType::GameOver, clawLevelEngine) {}
	void Logic(uint32_t elapsedTime) override;
};


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
		_planes.back()->init();
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
	//if (levelNumber == 1) GO::getPlayerPosition() = { 3586, 4859 };
	//if (levelNumber == 1) GO::getPlayerPosition() = { 8537, 4430 };
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
ClawLevelEngine::ClawLevelEngine(shared_ptr<ClawLevelEngineFields> fields)
	: _fields(fields)
{
	WindowManager::setBackgroundColor(_fields->_saveBgColor);
	WindowManager::setWindowScale(_fields->_saveWindowScale);

	init();
}
ClawLevelEngine::~ClawLevelEngine() { delete _state; }

void ClawLevelEngine::init()
{
	_holeRadius = 0;
	_state = DBG_NEW PlayState(this);
	_wrapDestination = {};
	_wrapCoverTop = 0;
	_isBossWarp = false;
	_bossWarpX = 0;
	_gameOverTimeCounter = 0;
	_nextState = nullptr;

	for (shared_ptr<LevelPlane>& pln : _fields->_planes)
		_elementsList.push_back(pln.get());
	_elementsList.push_back(_fields->_hud);

	WindowManager::setWindowOffset(_fields->actionPlane->position);
}
void ClawLevelEngine::playerEnterWrap(Warp* destinationWarp)
{
	switchState(DBG_NEW WrapCloseState(this));
	_wrapCoverTop = WindowManager::getCameraSize().height;
	_wrapDestination = destinationWarp->getDestination();
	_isBossWarp = destinationWarp->isBossWarp();
	_bossWarpX = destinationWarp->position.x;
}
float ClawLevelEngine::getInitialHoleRadius() const {
	const D2D1_SIZE_F wndRealSz = WindowManager::getRealSize();
	const float initialHoleRadius = max(wndRealSz.width, wndRealSz.height) / 2;
	return initialHoleRadius;
}
void ClawLevelEngine::switchState(ClawLevelEngineState* newState) {
	//delete _state;
	//_state = newState;
	_nextState = newState;
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

	switch (_state->getType())
	{
	case ClawLevelEngineState::StateType::DeathClose:
	case ClawLevelEngineState::StateType::DeathOpen:
		WindowManager::drawHole(GO::getPlayerPosition(), _holeRadius);
		break;

	case ClawLevelEngineState::StateType::WrapClose:
	case ClawLevelEngineState::StateType::WrapOpen:
		WindowManager::drawWrapCover(_wrapCoverTop);
		break;

	case ClawLevelEngineState::StateType::GameOver:
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


void PlayState::Logic(uint32_t elapsedTime) {
	_clawLevelEngine->BaseEngine::Logic(elapsedTime);
	for (shared_ptr<LevelPlane>& p : _clawLevelEngine->_fields->_planes)
		p->position = _clawLevelEngine->_fields->actionPlane->position;

	_clawLevelEngine->_fields->_planes.back()->isVisible = SavedDataManager::settings.frontLayer; // hide/display front plane

	if (player->isFinishDeathAnimation() && player->hasLives())
	{
		if (player->isSpikeDeath())
		{
			_clawLevelEngine->_holeRadius = _clawLevelEngine->getInitialHoleRadius();
			AssetsManager::playWavFile("GAME/SOUNDS/CIRCLEFADE.WAV");
			_clawLevelEngine->switchState(DBG_NEW DeathCloseState(_clawLevelEngine));
		}
		else //if (player->isFallDeath())
		{
			_clawLevelEngine->switchState(DBG_NEW DeathFallState(_clawLevelEngine));
		}
	}
	else
	{
		if (player->isFinishLevel())
		{
			_clawLevelEngine->changeEngine<LevelEndEngine>(_clawLevelEngine->_fields->_wwd->levelNumber, player->getCollectedTreasures());
		}
		else if (!player->hasLives())
		{
			if (player->isFinishDeathAnimation())
			{
				_clawLevelEngine->_gameOverTimeCounter = 1500;
				_clawLevelEngine->switchState(DBG_NEW GameOverState(_clawLevelEngine));
			}
		}
	}
}
void DeathFallState::Logic(uint32_t elapsedTime) {
	GO::getPlayerPosition().y += CC_FALLDEATH_SPEED * elapsedTime;
	player->Logic(0); // update position of animation
	if (GO::getPlayerPosition().y - _clawLevelEngine->_fields->actionPlane->position.y > WindowManager::getCameraSize().height)
	{
		player->loseLife();
		_clawLevelEngine->_holeRadius = _clawLevelEngine->getInitialHoleRadius();
		AssetsManager::playWavFile("GAME/SOUNDS/CIRCLEFADE.WAV");
		_clawLevelEngine->switchState(DBG_NEW DeathCloseState(_clawLevelEngine));
	}
}
void DeathCloseState::Logic(uint32_t elapsedTime) {
	_clawLevelEngine->_holeRadius -= SCREEN_SPEED * elapsedTime;
	if (_clawLevelEngine->_holeRadius <= 0)
	{
		_clawLevelEngine->_holeRadius = 0;
		player->backToLife();

		// update position of camera (because player position changed)
		_clawLevelEngine->_fields->actionPlane->resetObjects();
		_clawLevelEngine->BaseEngine::Logic(elapsedTime);
		for (shared_ptr<LevelPlane>& p : _clawLevelEngine->_fields->_planes)
			p->position = _clawLevelEngine->_fields->actionPlane->position;

		AssetsManager::playWavFile("GAME/SOUNDS/FLAGWAVE.WAV");

		_clawLevelEngine->switchState(DBG_NEW DeathOpenState(_clawLevelEngine));
	}
}
void DeathOpenState::Logic(uint32_t elapsedTime) {
	_clawLevelEngine->_holeRadius += SCREEN_SPEED * elapsedTime;
	if (_clawLevelEngine->getInitialHoleRadius() < _clawLevelEngine->_holeRadius)
		_clawLevelEngine->switchState(DBG_NEW PlayState(_clawLevelEngine));
}
void WrapCloseState::Logic(uint32_t elapsedTime) {
	_clawLevelEngine->_wrapCoverTop -= SCREEN_SPEED * elapsedTime;
	if (_clawLevelEngine->_wrapCoverTop <= 0)
	{
		GO::getPlayerPosition() = _clawLevelEngine->_wrapDestination;
		player->speed = {}; // stop player
		if (_clawLevelEngine->_isBossWarp)
		{
			_clawLevelEngine->_fields->actionPlane->playerEnterToBoss(_clawLevelEngine->_bossWarpX);
			player->startPosition = _clawLevelEngine->_wrapDestination;
		}
		player->Logic(0); // update position of animation

		// update position of camera (because player position changed)
		_clawLevelEngine->_fields->actionPlane->updatePosition();
		for (shared_ptr<LevelPlane>& p : _clawLevelEngine->_fields->_planes)
			p->position = _clawLevelEngine->_fields->actionPlane->position;

		_clawLevelEngine->switchState(DBG_NEW WrapOpenState(_clawLevelEngine));
	}
}
void WrapOpenState::Logic(uint32_t elapsedTime) {
	_clawLevelEngine->_wrapCoverTop -= SCREEN_SPEED * elapsedTime;
	if (_clawLevelEngine->_wrapCoverTop < -WindowManager::getCameraSize().height)
		_clawLevelEngine->switchState(DBG_NEW PlayState(_clawLevelEngine));
}
void GameOverState::Logic(uint32_t elapsedTime) {
	_clawLevelEngine->_gameOverTimeCounter -= elapsedTime;
	if (_clawLevelEngine->_gameOverTimeCounter <= 0)
	{
		_clawLevelEngine->_gameOverTimeCounter = 0;
		MenuEngine::setMainMenu();
		_clawLevelEngine->changeEngine<MenuEngine>();
	}
}
