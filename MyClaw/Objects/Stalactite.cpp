#include "Stalactite.h"
#include "Player.h"


Stalactite::Stalactite(const WwdObject& obj)
	: Projectile(obj, "") // the stalactite behave like a projectile
	, _initialPosition({ (float)obj.x, (float)obj.y })
{
	_ani = AssetsManager::createCopyAnimationFromDirectory(PathManager::getImageSetPath(obj.imageSet), false, 50);

	_activityArea.left = (float)obj.userRect1.left;
	_activityArea.top = (float)obj.userRect1.top;
	_activityArea.right = (float)obj.userRect1.right;
	_activityArea.bottom = (float)obj.userRect1.bottom;

	Reset();
}

void Stalactite::Logic(uint32_t elapsedTime)
{
	if (_ani->isFinishAnimation())
		return; // the object is not used (it finished its job for now)

	if (speed.y == 0)
	{
		if (_activityArea.intersects(player->GetRect()))
			speed.y = 0.25f;
	}
	else
	{
		speed.y += GRAVITY * elapsedTime;
		position.y += speed.y * elapsedTime;
	}

	_ani->position = position;
	_ani->Logic(elapsedTime);
}

void Stalactite::Draw()
{
	if (!_ani->isFinishAnimation())
		_ani->Draw();
}

void Stalactite::Reset()
{
	_ani->reset();
	_ani->updateFrames = false;
	_ani->loopAni = false;
	speed = {};
	position = _initialPosition;
}

void Stalactite::stopFalling(float collisionSize)
{
	_ani->updateFrames = true;
}

int Stalactite::getDamage() const
{
	return 10;
}
