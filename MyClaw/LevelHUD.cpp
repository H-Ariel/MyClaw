#include "LevelHUD.h"
#include "AssetsManager.h"
#include "WindowManager.h"


LevelHUD::LevelHUD(const Player* player, const D2D1_POINT_2F& offset)
	: _player(player), _offset(offset)
{
	_chest = AssetsManager::createAnimationFromDirectory("GAME/IMAGES/INTERFACE/TREASURECHEST", 125, false);
	_health = AssetsManager::createAnimationFromDirectory("GAME/IMAGES/INTERFACE/HEALTHHEART", 150, false);
	_weaponAni[ClawProjectile::Types::Pistol] = AssetsManager::loadAnimation("GAME/ANIS/INTERFACE/PISTOL.ANI");
	_weaponAni[ClawProjectile::Types::Magic] = AssetsManager::loadAnimation("GAME/ANIS/INTERFACE/MAGIC.ANI");
	_weaponAni[ClawProjectile::Types::Dynamite] = AssetsManager::loadAnimation("GAME/ANIS/INTERFACE/DYNAMITE.ANI");
	_lives = AssetsManager::loadAnimation("GAME/ANIS/INTERFACE/LIVES.ANI");
	_stopwatch = AssetsManager::createAnimationFromDirectory("GAME/IMAGES/INTERFACE/STOPWATCH", 125, false);

	char imgPath[44] = {};
	for (uint8_t i = 0; i <= 9; i++)
	{
		sprintf(imgPath, "GAME/IMAGES/INTERFACE/HEALTHNUMBERS/%03d.PID", i);
		_healthNumbers[i] = AssetsManager::loadImage(imgPath);
		sprintf(imgPath, "GAME/IMAGES/INTERFACE/SMALLNUMBERS/%03d.PID", i);
		_smallNumbers[i] = AssetsManager::loadImage(imgPath);
		sprintf(imgPath, "GAME/IMAGES/INTERFACE/SCORENUMBERS/%03d.PID", i);
		_scoreNumbers[i] = AssetsManager::loadImage(imgPath);
	}
}

void LevelHUD::Draw()
{
	D2D1_POINT_2F pos = {};
	const float winWidth = WindowManager::getSize().width - 15 / WindowManager::PixelSize;
	pos.y = 20 + _offset.y;

	_chest->position = { 20 + _offset.x, pos.y };
	_chest->updateImageData();
	_chest->Draw();
	
	float powerupLeftTime = _player->getPowerupLeftTime() / 1000.f;
	if (powerupLeftTime > 0)
	{
		_stopwatch->position = { 20 + _offset.x, pos.y + 30 };
		_stopwatch->updateImageData();
		_stopwatch->Draw();
		drawNumbers((uint32_t)powerupLeftTime, 3, _scoreNumbers, { 50 + _offset.x, 50 }, true);
	}

	for (shared_ptr<Animation> ani : { _health, _weaponAni[_player->getCurrentWeapon()], _lives })
	{
		Rectangle2D rc = ani->GetRect();
		ani->position = { winWidth - (rc.right - rc.left) / 2 + _offset.x, pos.y };
		ani->updateImageData();
		ani->Draw();
		pos.y += 30;
	}

	pos.x = winWidth - 35 + _offset.x;
	pos.y = 17; drawNumbers(_player->getHealthAmount(), 3, _healthNumbers, pos);
	pos.y = 52; drawNumbers(_player->getWeaponAmount(), 2, _smallNumbers, pos);
	pos.y = 81; drawNumbers(_player->getLivesAmount(), 1, _smallNumbers, pos);
	drawNumbers(_player->getScore(), 8, _scoreNumbers, { 50 + _offset.x, 20 }, true);
}

void LevelHUD::drawNumbers(uint32_t amount, int8_t numOfDigits, shared_ptr<UIBaseImage> const numArr[], D2D1_POINT_2F pos, bool isScore) const
{
	shared_ptr<UIBaseImage> img;
	char str[9];
	sprintf(str, "%08d", amount);
	if (!isScore) _strrev(str);
	else {
		char tmp[9];
		strcpy(tmp, str + 8 - numOfDigits);
		strcpy(str, tmp);
	}
	
	pos.y += _offset.y;
	
	for (int8_t i = 0; i < numOfDigits; i++)
	{
		img = numArr[str[i] - '0'];
		img->position = pos;
		img->Draw();
		if (isScore) pos.x += img->size.width;
		else pos.x -= img->size.width;
	}
}
