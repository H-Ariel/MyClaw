#include "LevelHUD.h"
#include "GameEngine/WindowManager.h"
#include "ActionPlane.h"


LevelHUD::LevelHUD(const D2D1_POINT_2F* windowOffset)
	: _windowOffset(windowOffset)
{
	_chest = AssetsManager::createAnimationFromDirectory("GAME/IMAGES/INTERFACE/TREASURECHEST");
	_health = AssetsManager::createAnimationFromDirectory("GAME/IMAGES/INTERFACE/HEALTHHEART", false, 150);
	_weaponAni[ClawProjectile::Types::Pistol] = AssetsManager::loadAnimation("GAME/ANIS/INTERFACE/PISTOL.ANI");
	_weaponAni[ClawProjectile::Types::Magic] = AssetsManager::loadAnimation("GAME/ANIS/INTERFACE/MAGIC.ANI");
	_weaponAni[ClawProjectile::Types::Dynamite] = AssetsManager::loadAnimation("GAME/ANIS/INTERFACE/DYNAMITE.ANI");
	_lives = AssetsManager::loadAnimation("GAME/ANIS/INTERFACE/LIVES.ANI");
	_stopwatch = AssetsManager::createAnimationFromDirectory("GAME/IMAGES/INTERFACE/STOPWATCH");
	_bossBar = AssetsManager::loadImage("GAME/IMAGES/BOSSBAR/001.PID");

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

	_bossHealth = ActionPlane::getBossHealth();
}

void LevelHUD::Draw()
{
	WindowManager::setWindowOffset(nullptr); // reset the offset


	Rectangle2D rc;
	D2D1_SIZE_F wndSz = WindowManager::getSize();

	_chest->position = { 20 , 20 };
	_chest->Draw();
	rc = _health->GetRect();
	_health->position = { wndSz.width - 32, 20 };
	_health->Draw();
	shared_ptr<UIAnimation> ani = _weaponAni[BasePlaneObject::player->getCurrentWeapon()];
	rc = ani->GetRect();
	ani->position = { wndSz.width - (rc.right - rc.left) / 2, 50 };
	ani->Draw();
	rc = _lives->GetRect();
	_lives->position = { wndSz.width - 18, 80 };
	_lives->Draw();

	float powerupLeftTime = BasePlaneObject::player->getPowerupLeftTime() / 1000.f;
	if (powerupLeftTime > 0)
	{
		_stopwatch->position = { 20, 20 + 30 };
		_stopwatch->Draw();
		drawNumbers((uint32_t)powerupLeftTime, 3, _scoreNumbers, 68, 50);
	}

	float posX = wndSz.width - 32;
	drawNumbers(BasePlaneObject::player->getHealth(), 3, _healthNumbers, posX, 17);
	drawNumbers(BasePlaneObject::player->getWeaponAmount(), 2, _smallNumbers, posX, 52);
	drawNumbers(BasePlaneObject::player->getLivesAmount(), 1, _smallNumbers, posX, 81);
	drawNumbers(BasePlaneObject::player->getScore(), 8, _scoreNumbers, 128, 20);

	if (ActionPlane::isInBoss())
	{
		_bossBar->position.x = 0.5f * wndSz.width;
		_bossBar->position.y = 0.9f * wndSz.height;
		_bossBar->Draw();

		// find the health bar rect (inside the boss bar frame)
		rc = _bossBar->GetRect();
		rc.top += 12;
		rc.bottom -= 11;
		rc.left += 18;
		rc.right -= 18;
		float width = rc.right - rc.left;
		float percent = (float)ActionPlane::getBossHealth() / _bossHealth;
		rc.right = rc.left + width * percent; // the right side decreases as the boss health decreases

		WindowManager::fillRect(rc, ColorF::Red); // draw the health bar
	}


	WindowManager::setWindowOffset(_windowOffset); // restore the offset
}

void LevelHUD::drawNumbers(uint32_t amount, int numOfDigits, shared_ptr<UIBaseImage> const numArr[],
	float posX, float posY) const
{
	shared_ptr<UIBaseImage> img;
	float width = numArr[0]->size.width;
	for (numOfDigits -= 1; numOfDigits >= 0; numOfDigits--)
	{
		img = numArr[amount % 10];
		img->position = { posX, posY };
		img->Draw();
		posX -= width;
		amount /= 10;
	}
}
