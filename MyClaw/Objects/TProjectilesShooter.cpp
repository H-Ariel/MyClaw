#include "TProjectilesShooter.h"
#include "../Player.h"
#include "../Assets-Managers/AssetsManager.h"
#include "../ActionPlane.h"
#include "EnemyProjectile.h"


#define OFFSET_X 32.f
#define OFFSET_Y 32.f


// TODO: fix TProjectilesShooter for level 14

TProjectilesShooter::TProjectilesShooter(const WwdObject& obj)
	: BaseStaticPlaneObject(obj), _maxRestTime(obj.speed > 100 ? obj.speed : 500),
	_damage(obj.damage > 0 ? obj.damage : 5), speed({ obj.speedX / 1000.f, obj.speedY / 1000.f }),
	_restTime(0), _offset({})
{
	// 1=right, 2=left, 3=down, 4=up   NOTE: only for levels 9,10 !!!
	switch (obj.userValue1)
	{
	case 1: myMemCpy(_offset.x, -OFFSET_X); break;
	case 2: myMemCpy(_offset.x, OFFSET_X); myMemCpy(speed.x, -speed.x); break;
	case 3: myMemCpy(_offset.y, -OFFSET_Y); break;
	case 4: myMemCpy(_offset.y, OFFSET_Y); myMemCpy(speed.y, -speed.y); break;
	}

	char frame[9];

	if (PathManager::getImageSetPath("LEVEL_") == "LEVEL14/IMAGES/") // in level 14 the projectiles have animations
	{
		sprintf(frame, "/%d", obj.userValue1);
		_projectileAni = AssetsManager::createAnimationFromDirectory(PathManager::getImageSetPath("LEVEL_PROJECTILES") + frame, 100, false);
	}
	else // levels 9,10
	{
		sprintf(frame, "/%03d.PID", obj.userValue1);
		_projectileAni = AssetsManager::createAnimationFromFromPidImage(PathManager::getImageSetPath("LEVEL_PROJECTILES") + frame);
	}
	
	vector<Animation::FrameData*> imgs = AssetsManager::createAnimationFromDirectory(PathManager::getImageSetPath(obj.imageSet), 100, false)->getImagesList();
	vector<Animation::FrameData*> newImgs;
	newImgs.push_back(DBG_NEW Animation::FrameData("", 0));
	newImgs.insert(newImgs.end(), imgs.begin(), imgs.end());
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
		// shoot when finish display animation 
		ActionPlane::addPlaneObject(DBG_NEW TProjectile(_projectileAni->getCopy(),
			_damage, speed, { position.x + _offset.x,position.y + _offset.y }));
		_restTime = _maxRestTime;
		_ani->reset();
		_ani->updateFrames = false;
	}
}
