#include "PowderKeg.h"
#include "Projectile.h"
#include "../AssetsManager.h"
#include "../PathManager.h"
#include "../Player.h"
#include "../CollisionDistances.h"
#include "../ActionPlane.h"


PowderKeg::PowderKeg(const WwdObject& obj, Player* player)
	: BaseDynamicPlaneObject(obj, player), _imageSet(obj.imageSet), _state(State::Stand)
{
	_ani = AssetsManager::createAnimationFromFromPidImage(PathManager::getImageSetPath(_imageSet) + "/001.PID")->getCopy();
}

void PowderKeg::Logic(uint32_t elapsedTime)
{
	if (shouldMakeExplos())
	{
		_state = State::Explos;
		_ani = AssetsManager::loadCopyAnimation(PathManager::getAnimationSetPath(_imageSet) + "/EXPLODE.ANI");
		_ani->loopAni = false;
	}

	if (_state == State::Explos)
	{
		removeObject = _ani->isFinishAnimation();
	}

	if (_state == State::Thrown)
	{
		position.x += _speed.x * elapsedTime;
		position.y += _speed.y * elapsedTime;
		_speed.y += GRAVITY * elapsedTime;
	}

	_ani->position = position;
	_ani->Logic(elapsedTime);
}
int8_t PowderKeg::getDamage() const
{
	if (_state == State::Explos && !_ani->isPassedHalf())
		return 15;
	return 0;
}

bool PowderKeg::shouldMakeExplos()
{
	if (_state == State::Stand)
	{
		D2D1_RECT_F thisRc = GetRect();
		bool isClawBullet;

		for (Projectile* p : ActionPlane::getProjectiles())
		{
			if ((isClawBullet = p->isClawBullet()) || (p->isClawDynamite() && p->getDamage() > 0)) // ATTENTION: =, not ==
			{
				if (CollisionDistances::isCollision(thisRc, p->GetRect()))
				{
					if (isClawBullet)
						p->removeObject = true;
					return true;
				}
			}
		}
		for (PowderKeg* p : ActionPlane::getPowderKegs())
		{
			if (p->_state == State::Explos && CollisionDistances::isCollision(thisRc, p->GetRect()))
			{
				return true;
			}
		}
	}
	else if (_state == State::Thrown)
	{
		if (_speed.x == 0 && _speed.y == 0)
		{
			return true;
		}
	}

	return false;
}

void PowderKeg::raise()
{
	_state = State::Raised;
	_ani = AssetsManager::createAnimationFromFromPidImage(PathManager::getImageSetPath(_imageSet) + "/014.PID")->getCopy();
}
void PowderKeg::thrown(bool forward)
{
	_state = State::Thrown;
	_ani = AssetsManager::loadCopyAnimation(PathManager::getAnimationSetPath(_imageSet) + "/THROWN.ANI");
	_ani->loopAni = false;

	_speed.x = forward ? 0.35f : -0.35f;
	_speed.y = -0.40f;
}
void PowderKeg::fall()
{
	_state = State::Thrown;
	_ani = AssetsManager::loadCopyAnimation(PathManager::getAnimationSetPath(_imageSet) + "/THROWN.ANI");
	_ani->loopAni = false;
	_speed.y = 0.01f; // almost no speed
}

void PowderKeg::stopFalling		(float collisionSize) { _speed = {}; position.y -= collisionSize; }
void PowderKeg::stopMovingLeft	(float collisionSize) { _speed = {}; position.x += collisionSize; }
void PowderKeg::stopMovingRight	(float collisionSize) { _speed = {}; position.x -= collisionSize; }
void PowderKeg::bounceTop() { _speed.y = abs(_speed.y); }
