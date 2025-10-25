#include "TProjectilesShooter.h"
#include "../GlobalObjects.h"
#include "Projectiles/TProjectile.h"


constexpr float OFFSET_X = TILE_SIZE / 2.f;
constexpr float OFFSET_Y = TILE_SIZE / 2.f;


TProjectilesShooter::TProjectilesShooter(const WwdObject& obj, int levelNumber)
	: BaseStaticPlaneObject(obj, false), _maxRestTime(obj.speed > 100 ? obj.speed : 500),
	_damage(obj.damage > 0 ? obj.damage : 5), _projSpeed({ obj.speedX / 1000.f, obj.speedY / 1000.f }),
	_offset({}), _projIsOut(false)
{
	char frame[9];

	if (levelNumber == 14) // in level 14 the projectiles have animations
	{
		if (obj.userValue2 == 1) // shoot from right to left
		{
			myMemCpy(_projSpeed.x, -_projSpeed.x);
		}

		sprintf(frame, "/%d", obj.userValue2);
		_projectileAni = AssetsManager::getAnimationFromDirectory(PathManager::getImageSetPath("LEVEL_PROJECTILES") + frame);

		_shootIndex = 1;
	}
	else // levels 9,10
	{
		// 1=right, 2=left, 3=down, 4=up
		switch (obj.userValue1)
		{
		case 1: myMemCpy(_offset.x, -OFFSET_X); break;
		case 2: myMemCpy(_offset.x, OFFSET_X); myMemCpy(_projSpeed.x, -_projSpeed.x); break;
		case 3: myMemCpy(_offset.y, -18.f); break;
		case 4: myMemCpy(_offset.y, 22.f); myMemCpy(_projSpeed.y, -_projSpeed.y); break;
		}

		sprintf(frame, "/%03d.PID", obj.userValue1);
		_projectileAni = AssetsManager::getAnimationFromPidImage(PathManager::getImageSetPath("LEVEL_PROJECTILES") + frame);

		_shootIndex = 5;
	}

	vector<UIAnimation::FrameData*> newImgs;
	newImgs.push_back(DBG_NEW UIAnimation::FrameData(make_shared<UIBaseImage>(nullptr))); // insert empty image at end
	newImgs += AssetsManager::getAnimationFromDirectory(PathManager::getImageSetPath(obj.imageSet))->getFramesList();
	_ani = make_shared<UIAnimation>(newImgs);
	_ani->updateFrames = false;

	myMemCpy(_objRc, Rectangle2D((float)obj.minX, (float)obj.minY, (float)obj.maxX, (float)obj.maxY));
}

void TProjectilesShooter::Logic(uint32_t elapsedTime)
{
	if (_restTimer.isFinished() && _objRc.intersects(GO::getPlayerRect()))
	{
		_ani->updateFrames = true;
	}

	_ani->Logic(elapsedTime);

	if (_ani->isFinishAnimation())
	{
		_restTimer.reset(_maxRestTime);
		addTimer(&_restTimer);
		_ani->reset();
		_ani->updateFrames = false;
		_projIsOut = false;
	}
	else if (_shootIndex == _ani->getFrameNumber() && !_projIsOut)
	{
		// shoot at middle of animation
		GO::addObjectToActionPlane(DBG_NEW TProjectile(_projectileAni->getCopy(),
			_damage, _projSpeed, { position.x + _offset.x, position.y + _offset.y }));
		_projIsOut = true;
	}
}
