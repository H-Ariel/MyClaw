#include "TProjectilesShooter.h"
#include "../Player.h"
#include "../Assets-Managers/AssetsManager.h"
#include "../ActionPlane.h"
#include "EnemyProjectile.h"


#define OFFSET_X 32.f
#define OFFSET_Y 32.f


TProjectilesShooter::TProjectilesShooter(const WwdObject& obj, int levelNumber)
	: BaseStaticPlaneObject(obj), _maxRestTime(obj.speed > 100 ? obj.speed : 500),
	_damage(obj.damage > 0 ? obj.damage : 5), _projSpeed({ obj.speedX / 1000.f, obj.speedY / 1000.f }),
	_restTime(0), _offset({}), _projIsOut(false)
{
	char frame[9];

	if (levelNumber == 14) // in level 14 the projectiles have animations
	{
		if (obj.userValue2 == 1)
		{
			(float&)_projSpeed.x = -_projSpeed.x;
			(float&)_offset.x = -OFFSET_X * 2;
		}
		else
		{
			(float&)_offset.x = OFFSET_X * 2;
		}

		sprintf(frame, "/%d", obj.userValue2);
		_projectileAni = AssetsManager::createAnimationFromDirectory(PathManager::getImageSetPath("LEVEL_PROJECTILES") + frame);
	}
	else // levels 9,10
	{
		// 1=right, 2=left, 3=down, 4=up
		switch (obj.userValue1)
		{
		case 1: myMemCpy(_offset.x, -OFFSET_X); break;
		case 2: myMemCpy(_offset.x, OFFSET_X); myMemCpy(_projSpeed.x, -_projSpeed.x); break;
		case 3: myMemCpy(_offset.y, -OFFSET_Y); break;
		case 4: myMemCpy(_offset.y, OFFSET_Y); myMemCpy(_projSpeed.y, -_projSpeed.y); break;
		}

		sprintf(frame, "/%03d.PID", obj.userValue1);
		_projectileAni = AssetsManager::createAnimationFromPidImage(PathManager::getImageSetPath("LEVEL_PROJECTILES") + frame);
	}
	
	vector<Animation::FrameData*> imgs = AssetsManager::createAnimationFromDirectory(PathManager::getImageSetPath(obj.imageSet))->getImagesList(), newImgs;
	newImgs.push_back(DBG_NEW Animation::FrameData("", 0)); // insert empty image at end
	newImgs += imgs;
	_ani = allocNewSharedPtr<Animation>(newImgs);
	_ani->updateFrames = false;

	myMemCpy(_objRc, Rectangle2D((float)obj.minX, (float)obj.minY, (float)obj.maxX, (float)obj.maxY));
}

void TProjectilesShooter::Logic(uint32_t elapsedTime)
{
	if (_restTime > 0)
		_restTime -= elapsedTime;

	if (_restTime <= 0 && _objRc.intersects(player->GetRect()))
	{
		_ani->updateFrames = true;
	}

	_ani->Logic(elapsedTime);

	if (_ani->isFinishAnimation())
	{
		_restTime = _maxRestTime;
		_ani->reset();
		_ani->updateFrames = false;
		_projIsOut = false;
	}
	else if (_ani->isPassedHalf() && !_projIsOut)
	{
		// shoot at middle of animation
		ActionPlane::addPlaneObject(DBG_NEW TProjectile(_projectileAni->getCopy(),
			_damage, _projSpeed, { position.x + _offset.x, position.y + _offset.y }));
		_projIsOut = true;
	}
}
