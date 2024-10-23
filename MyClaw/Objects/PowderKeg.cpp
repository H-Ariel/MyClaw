#include "PowderKeg.h"
#include "Projectile.h"
#include "../Player.h"
#include "../ActionPlane.h"


PowderKeg::PowderKeg(const WwdObject& obj)
	: BaseDynamicPlaneObject(obj), _imageSet(obj.imageSet), _state(State::Stand), _isPlaySound(false)
{
	_ani = AssetsManager::createAnimationFromPidImage(PathManager::getImageSetPath(_imageSet) + "/001.PID")->getCopy();
}

void PowderKeg::Logic(uint32_t elapsedTime)
{
	if (shouldMakeExplos())
	{
		_state = State::Explos;
		_ani = AssetsManager::loadCopyAnimation(PathManager::getAnimationSetPath(_imageSet) + "/EXPLODE.ANI", _imageSet);
		_ani->loopAni = false;
	}

	if (_state == State::Thrown || _state == State::Stand)
	{
		_ani->Logic(elapsedTime);
		position.x += speed.x * elapsedTime;
		speed.y += GRAVITY * elapsedTime;
		position.y += speed.y * elapsedTime;
	}
	else if (_state == State::Explos)
	{
		_ani->Logic(elapsedTime);
		removeObject = _ani->isFinishAnimation();
		if (!_isPlaySound)
		{
			AssetsManager::playWavFile("GAME/SOUNDS/KEGBOOM.WAV");
			_isPlaySound = true;
		}
	}
}

void PowderKeg::stopFalling(float collisionSize)
{
	speed = {};
	if (_state != State::Explos)
		position.y -= collisionSize;
}
void PowderKeg::stopMovingLeft(float collisionSize)
{
	speed.x = -speed.x;
	speed.y = 0;
	if (_state != State::Explos)
		position.x += collisionSize;
}
void PowderKeg::stopMovingRight(float collisionSize)
{
	speed.x = -speed.x;
	speed.y = 0;
	if (_state != State::Explos)
		position.x -= collisionSize;
}
void PowderKeg::bounceTop()
{
	speed.y = abs(speed.y);
}

int PowderKeg::getDamage() const
{
	if (_state == State::Explos && !_ani->isPassedHalf())
		return 15;
	return 0;
}

bool PowderKeg::raise()
{
	if (_state != State::Stand)
		return false;

	_state = State::Raised;
	_ani = AssetsManager::createCopyAnimationFromPidImage(PathManager::getImageSetPath(_imageSet) + "/014.PID");

	return true;
}
void PowderKeg::thrown(bool forward)
{
	_state = State::Thrown;
	_ani = AssetsManager::loadCopyAnimation(PathManager::getAnimationSetPath(_imageSet) + "/THROWN.ANI", _imageSet);
	_ani->loopAni = false;

	speed.x = forward ? 0.35f : -0.35f;
	speed.y = -0.40f;
}
void PowderKeg::fall()
{
	_state = State::Thrown;
	_ani = AssetsManager::loadCopyAnimation(PathManager::getAnimationSetPath(_imageSet) + "/THROWN.ANI", _imageSet);
	_ani->loopAni = false;
	speed.y = 0.01f; // almost no speed
}
bool PowderKeg::isStartExplode() const
{
	return _state == State::Explos && _ani->getFrameNumber() == 1;
}
bool PowderKeg::isExplode() const
{
	return _state == State::Explos;
}

bool PowderKeg::shouldMakeExplos()
{
	if (_state == State::Thrown && speed.x == 0 && speed.y == 0)
	{
		return true;
	}
	else if (_state == State::Stand)
	{
		Rectangle2D thisRc = GetRect();
		bool isClawBullet;

		for (Projectile* p : actionPlane->getProjectiles())
		{
			if (
				((isClawBullet = p->isClawBullet()) // ATTENTION: =, not ==
					|| (p->isClawDynamite() && p->getDamage() > 0))
				&& thisRc.intersects(p->GetRect()))
			{
				if (isClawBullet)
					p->removeObject = true;
				return true;
			}
		}
		for (PowderKeg* p : actionPlane->getPowderKegs())
		{
			if (p->_state == State::Explos && thisRc.intersects(p->GetRect()))
			{
				return true;
			}
		}
	}

	return false;
}
