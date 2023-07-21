#include "TProjectilesShooter.h"
#include "../Player.h"
#include "../AssetsManager.h"
#include "../ActionPlane.h"


#define OFFSET_X 32.f
#define OFFSET_Y 64.f


TProjectilesShooter::TProjectilesShooter(const WwdObject& obj, Player* player)
	: BaseStaticPlaneObject(obj, player), _maxRestTime(obj.speed > 100 ? obj.speed : 500),
	_damage(obj.damage > 0 ? obj.damage : 5), _speed({ obj.speedX / 1000.f, obj.speedY / 1000.f }),
	_restTime(0), _offset({})
{
	// 1=right, 2=left, 3=down, 4=up
	switch (obj.userValue1)
	{
	case 1: myMemCpy(_offset.x, -OFFSET_X); break;
	case 2: myMemCpy(_offset.x, OFFSET_X); myMemCpy(_speed.x, -_speed.x); break;
	case 3: myMemCpy(_offset.y, -OFFSET_Y); break;
	case 4: myMemCpy(_offset.y, OFFSET_Y); myMemCpy(_speed.y, -_speed.y); break;
	}

	char frame[9]; sprintf(frame, "/%03d.PID", obj.userValue1);
	_ani = AssetsManager::createAnimationFromFromPidImage(PathManager::getImageSetPath("LEVEL_PROJECTILES") + frame);

	myMemCpy(_objRc, Rectangle2D((float)obj.minX, (float)obj.minY, (float)obj.maxX, (float)obj.maxY));
}

void TProjectilesShooter::Logic(uint32_t elapsedTime)
{
	if (_restTime > 0)
		_restTime -= elapsedTime;

	if (_restTime <= 0 && _objRc.intersects(_player->GetRect()))
	{
		ActionPlane::addPlaneObject(DBG_NEW TProjectile(_ani, _damage,
			_speed, { position.x + _offset.x, position.y + _offset.y }));
		_restTime = _maxRestTime;
	}
}

void TProjectilesShooter::Draw()
{
}
