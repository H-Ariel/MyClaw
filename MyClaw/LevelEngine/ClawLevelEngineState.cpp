#include "ClawLevelEngineState.h"
#include "ActionPlane.h"
#include "GlobalObjects.h"
#include "GameEngine/WindowManager.h"
#include "Menu/LevelEndEngine.h"
#include "Menu/MenuEngine.h"


constexpr float SCREEN_SPEED = 0.5f; // speed of the screen when CC is died or teleported
constexpr float CC_FALLDEATH_SPEED = 0.7f; // speed of CC when he falls out the window
constexpr auto& player = GO::player;


void ClawLevelEngineState::Draw() { }


PlayState::PlayState(ClawLevelEngine* clawLevelEngine)
	: ClawLevelEngineState(clawLevelEngine)
{
	_clawLevelEngine->_fields->_planes.back()->isVisible = SavedDataManager::settings.frontLayer; // hide/display front plane
}
void PlayState::Logic(uint32_t elapsedTime)
{
	_clawLevelEngine->BaseEngine::Logic(elapsedTime);
	for (shared_ptr<LevelPlane>& p : _clawLevelEngine->_fields->_planes)
		p->position = _clawLevelEngine->_fields->actionPlane->position;

	if (player->isFinishDeathAnimation() && player->hasLives())
	{
		if (player->isSpikeDeath())
		{
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
				_clawLevelEngine->switchState(DBG_NEW GameOverState(_clawLevelEngine));
			}
		}
	}
}

void DeathFallState::Logic(uint32_t elapsedTime)
{
	GO::getPlayerPosition().y += CC_FALLDEATH_SPEED * elapsedTime;
	player->Logic(0); // update position of animation
	if (GO::getPlayerPosition().y - _clawLevelEngine->_fields->actionPlane->position.y > WindowManager::getCameraSize().height)
	{
		player->loseLife();
		AssetsManager::playWavFile("GAME/SOUNDS/CIRCLEFADE.WAV");
		_clawLevelEngine->switchState(DBG_NEW DeathCloseState(_clawLevelEngine));
	}
}

DeathCloseState::DeathCloseState(ClawLevelEngine* clawLevelEngine)
	: ClawLevelEngineState(clawLevelEngine)
{
	_holeRadius = _clawLevelEngine->getMaximalHoleRadius();
}
void DeathCloseState::Logic(uint32_t elapsedTime)
{
	_holeRadius -= SCREEN_SPEED * elapsedTime;
	if (_holeRadius <= 0)
	{
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
void DeathCloseState::Draw()
{
	WindowManager::drawHole(GO::getPlayerPosition(), _holeRadius);
}

void DeathOpenState::Logic(uint32_t elapsedTime)
{
	_holeRadius += SCREEN_SPEED * elapsedTime;
	if (_clawLevelEngine->getMaximalHoleRadius() < _holeRadius)
		_clawLevelEngine->switchState(DBG_NEW PlayState(_clawLevelEngine));
}
void DeathOpenState::Draw()
{
	WindowManager::drawHole(GO::getPlayerPosition(), _holeRadius);
}

WrapCloseState::WrapCloseState(ClawLevelEngine* clawLevelEngine, Warp* destinationWarp)
	: ClawLevelEngineState(clawLevelEngine), _wrapDestination(destinationWarp->getDestination()),
	_isBossWarp(destinationWarp->isBossWarp()), _bossWarpX(destinationWarp->position.x)
{
	_wrapCoverTop = WindowManager::getCameraSize().height;
}
void WrapCloseState::Logic(uint32_t elapsedTime)
{
	_wrapCoverTop -= SCREEN_SPEED * elapsedTime;
	if (_wrapCoverTop <= 0)
	{
		GO::getPlayerPosition() = _wrapDestination;
		player->speed = {}; // stop player
		if (_isBossWarp)
		{
			_clawLevelEngine->_fields->actionPlane->playerEnterToBoss(_bossWarpX);
			player->startPosition = _wrapDestination;
		}
		player->Logic(0); // update position of animation

		// update position of camera (because player position changed)
		_clawLevelEngine->_fields->actionPlane->updatePosition();
		for (shared_ptr<LevelPlane>& p : _clawLevelEngine->_fields->_planes)
			p->position = _clawLevelEngine->_fields->actionPlane->position;

		_clawLevelEngine->switchState(DBG_NEW WrapOpenState(_clawLevelEngine));
	}
}
void WrapCloseState::Draw()
{
	WindowManager::drawWrapCover(_wrapCoverTop);
}

void WrapOpenState::Logic(uint32_t elapsedTime)
{
	_wrapCoverTop -= SCREEN_SPEED * elapsedTime;
	if (_wrapCoverTop < -WindowManager::getCameraSize().height)
		_clawLevelEngine->switchState(DBG_NEW PlayState(_clawLevelEngine));
}
void WrapOpenState::Draw()
{
	WindowManager::drawWrapCover(_wrapCoverTop);
}

GameOverState::GameOverState(ClawLevelEngine* clawLevelEngine)
	: ClawLevelEngineState(clawLevelEngine), _gameOverTimeCounter(1500) { }
void GameOverState::Logic(uint32_t elapsedTime)
{
	_gameOverTimeCounter -= elapsedTime;
	if (_gameOverTimeCounter <= 0)
	{
		MenuEngine::setMainMenu();
		_clawLevelEngine->changeEngine<MenuEngine>();
	}
}
void GameOverState::Draw()
{
	const D2D1_SIZE_F wndSz = WindowManager::getCameraSize();
	shared_ptr<UIBaseImage> img = AssetsManager::loadImage("GAME/IMAGES/MESSAGES/004.PID");
	img->position.x = _clawLevelEngine->_fields->actionPlane->position.x + wndSz.width / 2;
	img->position.y = _clawLevelEngine->_fields->actionPlane->position.y + wndSz.height / 2;
	img->Draw();
}
