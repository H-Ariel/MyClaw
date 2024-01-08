#include "LevelHUD.h"
#include "GameEngine/WindowManager.h"
#include "ActionPlane.h"


int LevelHUD::_bossInitialHealth = 0;


LevelHUD::LevelHUD(const D2D1_POINT_2F& offset)
	: _offset(offset)
{
	_chest = AssetsManager::createAnimationFromDirectory("GAME/IMAGES/INTERFACE/TREASURECHEST");
	_health = AssetsManager::createAnimationFromDirectory("GAME/IMAGES/INTERFACE/HEALTHHEART", false, 150);
	_weaponAni[ClawProjectile::Types::Pistol] = AssetsManager::loadAnimation("GAME/ANIS/INTERFACE/PISTOL.ANI");
	_weaponAni[ClawProjectile::Types::Magic] = AssetsManager::loadAnimation("GAME/ANIS/INTERFACE/MAGIC.ANI");
	_weaponAni[ClawProjectile::Types::Dynamite] = AssetsManager::loadAnimation("GAME/ANIS/INTERFACE/DYNAMITE.ANI");
	_lives = AssetsManager::loadAnimation("GAME/ANIS/INTERFACE/LIVES.ANI");
	_stopwatch = AssetsManager::createAnimationFromDirectory("GAME/IMAGES/INTERFACE/STOPWATCH");
	_bossBar = AssetsManager::createAnimationFromPidImage("GAME/IMAGES/BOSSBAR/001.PID");

	char imgPath[44] = {};
	for (int i = 0; i <= 9; i++)
	{
		sprintf(imgPath, "GAME/IMAGES/INTERFACE/HEALTHNUMBERS/%03d.PID", i);
		_healthNumbers[i] = AssetsManager::loadImage(imgPath);
		sprintf(imgPath, "GAME/IMAGES/INTERFACE/SMALLNUMBERS/%03d.PID", i);
		_smallNumbers[i] = AssetsManager::loadImage(imgPath);
		sprintf(imgPath, "GAME/IMAGES/INTERFACE/SCORENUMBERS/%03d.PID", i);
		_scoreNumbers[i] = AssetsManager::loadImage(imgPath);
	}
}
LevelHUD::~LevelHUD()
{
	_bossInitialHealth = 0;
}

void LevelHUD::Draw()
{
	D2D1_POINT_2F pos = {};
	D2D1_SIZE_F wndSz = WindowManager::getSize();
	const float winWidth = wndSz.width - 15 / WindowManager::getPixelSize();
	pos.y = 20 + _offset.y;

	_chest->position = { 20 + _offset.x, pos.y };
	_chest->updateImageData();
	_chest->Draw();
	
	float powerupLeftTime = BasePlaneObject::player->getPowerupLeftTime() / 1000.f;
	if (powerupLeftTime > 0)
	{
		_stopwatch->position = { 20 + _offset.x, pos.y + 30 };
		_stopwatch->updateImageData();
		_stopwatch->Draw();
		drawNumbers((uint32_t)powerupLeftTime, 3, _scoreNumbers, { 50 + _offset.x, 50 }, true);
	}

	for (shared_ptr<Animation> ani : { _health, _weaponAni[BasePlaneObject::player->getCurrentWeapon()], _lives })
	{
		Rectangle2D rc = ani->GetRect();
		ani->position = { winWidth - (rc.right - rc.left) / 2 + _offset.x, pos.y };
		ani->updateImageData();
		ani->Draw();
		pos.y += 30;
	}

	pos.x = winWidth - 35 + _offset.x;
	pos.y = 17; drawNumbers(BasePlaneObject::player->getHealth(), 3, _healthNumbers, pos);
	pos.y = 52; drawNumbers(BasePlaneObject::player->getWeaponAmount(), 2, _smallNumbers, pos);
	pos.y = 81; drawNumbers(BasePlaneObject::player->getLivesAmount(), 1, _smallNumbers, pos);
	drawNumbers(BasePlaneObject::player->getScore(), 8, _scoreNumbers, { 50 + _offset.x, 20 }, true);

	if (ActionPlane::isInBoss())
	{
		_bossBar->position.x = _offset.x + 0.5f * wndSz.width;
		_bossBar->position.y = _offset.y + 0.9f * wndSz.height;
		_bossBar->updateImageData();
		_bossBar->Draw();

		// find the health bar rect (inside the boss bar frame)
		Rectangle2D rc = _bossBar->GetRect();
		rc.top += 12;
		rc.bottom -= 11;
		rc.left += 18;
		rc.right -= 18;

		float width = rc.right - rc.left;
		float percent = (float)ActionPlane::getBossHealth() / _bossInitialHealth;
		rc.right = rc.left + width * percent;

		WindowManager::fillRect(rc, ColorF::Red); // draw the health bar
	}
}

void LevelHUD::drawNumbers(uint32_t amount, int numOfDigits, shared_ptr<UIBaseImage> const numArr[], D2D1_POINT_2F pos, bool isScore) const
{
	shared_ptr<UIBaseImage> img;
	char str[9];
	sprintf(str, "%08d", amount);
	if (!isScore) _strrev(str);
	else {
		char tmp[9];
		strcpy(tmp, str + 8 - numOfDigits); // save only what we need
		strcpy(str, tmp);
	}
	
	pos.y += _offset.y;
	
	for (int i = 0; i < numOfDigits; i++)
	{
		img = numArr[str[i] - '0'];
		img->position = pos;
		img->Draw();
		if (isScore) pos.x += img->size.width;
		else pos.x -= img->size.width;
	}
}
