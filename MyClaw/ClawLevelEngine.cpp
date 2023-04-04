#include "ClawLevelEngine.h"
#include "AssetsManager.h"
#include "WindowManager.h"
#include "MenuEngine.h"


ClawLevelEngine::ClawLevelEngine(int8_t levelNumber)
	: _state(State::Play), saveBgColor(0), _levelNumber(levelNumber)
{
	_helpImage = DBG_NEW MenuBackgroundImage("STATES/HELP/SCREENS/HELP.PCX");
	_elementsList.push_back(_levelMap = DBG_NEW LevelMap(levelNumber));
	_elementsList.push_back(_hud = DBG_NEW LevelHUD(_player = _levelMap->getPlayer(), *_levelMap->getWindowOffset()));
	WindowManager::setWindowOffset(_levelMap->getWindowOffset());

	//if (levelNumber == 1) _player->position = { 17485, 1500 }; // END OF LEVEL
//	if (levelNumber == 1) _player->position = { 5775, 4347 };
//	if (levelNumber == 1) _player->position = { 9696, 772 };
//	if (levelNumber == 1) _player->position = { 5226, 4035 };
	if (levelNumber == 1) _player->position = { 2596,4155 };
//	if (levelNumber == 1) _player->position = { 4400, 4347 };
//	if (levelNumber == 1) _player->position = { 11039, 1851 };
//	if (levelNumber == 1) _player->position = { 2567, 4388 };
//	if (levelNumber == 2) _player->position = { 9196, 3958 };
//	if (levelNumber == 2) _player->position = { 16734, 1542 };
//	if (levelNumber == 2) _player->position = { 10881, 3382 };
//	if (levelNumber == 2) _player->position = { 593, 4086 };
//	if (levelNumber == 2) _player->position = { 17044, 3062 };
//	if (levelNumber == 2) _player->position = { 4596, 3958 };
	if (levelNumber == 2) _player->position = { 20070, 2092 }; // END OF LEVEL
//	if (levelNumber == 3) _player->position = { 23072, 6141 }; // ALMOST END OF LEVEL
	if (levelNumber == 3) _player->position = { 6080, 6224 };
//	if (levelNumber == 3) _player->position = { 11054, 8720 };
	if (levelNumber == 4) _player->position = { 16010, 1464 }; // END OF LEVEL
//	if (levelNumber == 4) _player->position = { 3909, 4832 };
//	if (levelNumber == 5) _player->position = { 9981, 3616 };
//	if (levelNumber == 5) _player->position = { 2620, 2848 };
	if (levelNumber == 5) _player->position = { 7189, 3488 };
//	if (levelNumber == 6) _player->position = { 2676, 5878 };
//	if (levelNumber == 6) _player->position = { 2036, 5878 };
	if (levelNumber == 6) _player->position = { 30685, 4179 }; // END OF LEVEL
//	if (levelNumber == 7) _player->position = { 3428, 7254 };
//	if (levelNumber == 7) _player->position = { 10203, 7388 };
	if (levelNumber == 7) _player->position = { 12316, 7388 };
//	if (levelNumber == 7) _player->position = { 25210, 7196 };
	if (levelNumber == 8) _player->position = { 4466, 5039 };
//	if (levelNumber == 9) _player->position = { 5210, 7776 };
//	if (levelNumber == 9) _player->position = { 4220, 6484 };
//	if (levelNumber == 9) _player->position = { 4699, 6548 };
//	if (levelNumber == 9) _player->position = { 7828, 7772 };
	if (levelNumber == 9) _player->position = { 1609, 7252 };
//	if (levelNumber == 11) _player->position = { 5116, 1360 };
//	if (levelNumber == 11) _player->position = { 3616, 1486 };
//	if (levelNumber == 11) _player->position = { 13350, 1119 };
	if (levelNumber == 11) _player->position = { 6837, 1360 };
}
ClawLevelEngine::~ClawLevelEngine()
{
	delete _helpImage;
	delete _hud;
	delete _levelMap;

	AssetsManager::clearLevelAssets(_levelNumber);
}

void ClawLevelEngine::Logic(uint32_t elapsedTime)
{
	BaseEngine::Logic(elapsedTime);

	if (_state == State::Play)
	{
		if (!_player->hasLives())
		{
			if (_player->isFinishDeathAnimation())
			{
				MessageBoxA(nullptr, "You died", "", 0);
				StopEngine = true;
			//	changeEngine<MenuEngine>();
			}
		}
		else if (_player->isFinishLevel())
		{
			changeEngine<LevelEndEngine>(_levelNumber);
		}
	}
}

void ClawLevelEngine::OnKeyUp(int key)
{
	if (key == 0xFF) return; // `Fn` key

	if (_state == State::Play)
	{
		if (key == VK_F1)
		{
			_state = State::Help;
			saveBgColor = WindowManager::getBackgroundColor();
			WindowManager::setBackgroundColor(ColorF::Black);
			_elementsList.clear();
			_elementsList.push_back(_helpImage);
			WindowManager::setWindowOffset(nullptr);
		}
		/*
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
		*/
		else
		{
			_player->keyUp(key);
		}
	}
	else // if (_state == State::Help)
	{
		_state = State::Play;
		WindowManager::setBackgroundColor(saveBgColor);
		_elementsList.clear();
		_elementsList.push_back(_levelMap);
		_elementsList.push_back(_hud);
		WindowManager::setWindowOffset(_levelMap->getWindowOffset());
	}
}
void ClawLevelEngine::OnKeyDown(int key)
{
	_player->keyDown(key);
}
