#include "ClawLevelEngineState.h"
#include "ActionPlane.h"
#include "GlobalObjects.h"
#include "GameEngine/WindowManager.h"
#include "ScreenEngines/LevelEndEngine/LevelEndEngine.h"
#include "ScreenEngines/MenuEngine/MenuEngine.h"


constexpr float SCREEN_SPEED = 0.5f; // speed of the screen when CC is died or teleported
constexpr float CC_FALLDEATH_SPEED = 0.7f; // speed of CC when he falls out the window
constexpr auto& player = GO::player;


void ClawLevelEngineState::Draw() {}


PlayState::PlayState(ClawLevelEngine* clawLevelEngine)
	: ClawLevelEngineState(clawLevelEngine)
{
	_levelEngine->_planes.back()->isVisible = SavedDataManager::settings.frontLayer; // hide/display front plane
}
void PlayState::Logic(uint32_t elapsedTime)
{
	_levelEngine->BaseEngine::Logic(elapsedTime);
	for (shared_ptr<LevelPlane>& p : _levelEngine->_planes)
		p->position = _levelEngine->actionPlane->position;

	if (player->isFinishDeathAnimation() && player->hasLives())
	{
		if (player->isSpikeDeath())
		{
			AssetsManager::playWavFile("GAME/SOUNDS/CIRCLEFADE.WAV");
			_levelEngine->switchState(DBG_NEW DeathCloseState(_levelEngine));
		}
		else //if (player->isFallDeath())
		{
			_levelEngine->switchState(DBG_NEW DeathFallState(_levelEngine));
		}
	}
	else
	{
		if (player->isFinishLevel())
		{
			_levelEngine->changeEngine(make_shared<LevelEndEngine>(_levelEngine->_wwd->levelNumber, player->getCollectedTreasures()));
		}
		else if (!player->hasLives())
		{
			if (player->isFinishDeathAnimation())
			{
				_levelEngine->switchState(DBG_NEW GameOverState(_levelEngine));
			}
		}
	}
}

void DeathFallState::Logic(uint32_t elapsedTime)
{
	GO::getPlayerPosition().y += CC_FALLDEATH_SPEED * elapsedTime;
	player->Logic(0); // update position of animation
	if (GO::getPlayerPosition().y - _levelEngine->actionPlane->position.y > WindowManager::getCameraSize().height)
	{
		player->loseLife();
		AssetsManager::playWavFile("GAME/SOUNDS/CIRCLEFADE.WAV");
		_levelEngine->switchState(DBG_NEW DeathCloseState(_levelEngine));
	}
}

DeathCloseState::DeathCloseState(ClawLevelEngine* clawLevelEngine)
	: ClawLevelEngineState(clawLevelEngine)
{
	_holeRadius = _levelEngine->getMaximalHoleRadius();
}
void DeathCloseState::Logic(uint32_t elapsedTime)
{
	_holeRadius -= SCREEN_SPEED * elapsedTime;
	if (_holeRadius <= 0)
	{
		player->backToLife();

		// update position of camera (because player position changed)
		_levelEngine->actionPlane->resetObjects();
		_levelEngine->BaseEngine::Logic(elapsedTime);
		for (shared_ptr<LevelPlane>& p : _levelEngine->_planes)
			p->position = _levelEngine->actionPlane->position;

		AssetsManager::playWavFile("GAME/SOUNDS/FLAGWAVE.WAV");

		_levelEngine->switchState(DBG_NEW DeathOpenState(_levelEngine));
	}
}
void DeathCloseState::Draw()
{
	WindowManager::drawHole(GO::getPlayerPosition(), _holeRadius);
}

void DeathOpenState::Logic(uint32_t elapsedTime)
{
	_holeRadius += SCREEN_SPEED * elapsedTime;
	if (_levelEngine->getMaximalHoleRadius() < _holeRadius)
		_levelEngine->switchState(DBG_NEW PlayState(_levelEngine));
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
			_levelEngine->actionPlane->playerEnterToBoss(_bossWarpX);
			player->startPosition = _wrapDestination;
		}
		player->Logic(0); // update position of animation

		// update position of camera (because player position changed)
		_levelEngine->actionPlane->updatePosition();
		for (shared_ptr<LevelPlane>& p : _levelEngine->_planes)
			p->position = _levelEngine->actionPlane->position;

		_levelEngine->switchState(DBG_NEW WrapOpenState(_levelEngine));
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
		_levelEngine->switchState(DBG_NEW PlayState(_levelEngine));
}
void WrapOpenState::Draw()
{
	WindowManager::drawWrapCover(_wrapCoverTop);
}

GameOverState::GameOverState(ClawLevelEngine* clawLevelEngine)
	: ClawLevelEngineState(clawLevelEngine), _gameOverTimeCounter(1500) {}
void GameOverState::Logic(uint32_t elapsedTime)
{
	_gameOverTimeCounter -= elapsedTime;
	if (_gameOverTimeCounter <= 0)
	{
		MenuEngine::setMainMenu();
		_levelEngine->changeEngine(make_shared<MenuEngine>());
	}
}
void GameOverState::Draw()
{
	const D2D1_SIZE_F wndSz = WindowManager::getCameraSize();
	shared_ptr<UIBaseImage> img = AssetsManager::loadImage("GAME/IMAGES/MESSAGES/004.PID");
	img->position.x = _levelEngine->actionPlane->position.x + wndSz.width / 2;
	img->position.y = _levelEngine->actionPlane->position.y + wndSz.height / 2;
	img->Draw();
}
