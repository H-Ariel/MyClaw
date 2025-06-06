#include "ClawProjectile.h"


ClawProjectile* ClawProjectile::createNew(Types type, const WwdObject& data)
{
	switch (type)
	{
	case Types::Pistol:		return DBG_NEW ClawProjectile(data, "GAME/IMAGES/BULLETS", type);
	case Types::Magic:		return DBG_NEW ClawProjectile(data, "GAME/IMAGES/MAGICCLAW", type);
	case Types::Dynamite:	return DBG_NEW ClawDynamite(data);
	case Types::FireSword:	return DBG_NEW ClawProjectile(data, "GAME/IMAGES/PROJECTILES/FIRESWORD", type);
	case Types::IceSword:	return DBG_NEW ClawProjectile(data, "GAME/IMAGES/PROJECTILES/ICESWORD", type);
	case Types::LightningSword:	return DBG_NEW ClawProjectile(data, "GAME/IMAGES/PROJECTILES/LIGHTNINGSWORD", type);
	}
	return nullptr;
}

ClawProjectile::ClawProjectile(const WwdObject& obj, const string& aniDirPath, Types type)
	: Projectile(obj, aniDirPath), type(type) {}

Rectangle2D ClawProjectile::GetRect()
{
	Rectangle2D rc = Projectile::GetRect();
	if (type == Types::FireSword || type == Types::IceSword || type == Types::LightningSword)
	{
		// ignore the tail of the projectile:
		if (_isMirrored) rc.right -= 64;
		else rc.left += 64;

		// ignore the border of the projectile:
		rc.top = position.y - 15;
		rc.bottom = position.y + 15;
	}
	return rc;
}

ClawDynamite::ClawDynamite(const WwdObject& obj)
	: ClawProjectile(obj, "GAME/ANIS/DYNAMITELIT.ANI", Types::Dynamite),
	_delayBeforeExplos(500), _state(State::Fly), _isPlaySound(false) {}

void ClawDynamite::Logic(uint32_t elapsedTime)
{
	switch (_state)
	{
	case State::Fly:
		position.x += speed.x * elapsedTime;
		speed.y += GRAVITY * elapsedTime;
		position.y += speed.y * elapsedTime;
		break;

	case State::Wait:
		_delayBeforeExplos -= elapsedTime;
		if (_delayBeforeExplos <= 0)
		{
			position.y -= 32;
			_state = State::Explos;
			_ani = AssetsManager::loadCopyAnimation("GAME/ANIS/DYNAMITEEXPLO.ANI");
			_ani->loopAni = false;
		}
		break;

	case State::Explos:
		removeObject = _ani->isFinishAnimation();
		if (!_isPlaySound)
		{
			AssetsManager::playWavFile("GAME/SOUNDS/KEGBOOM.WAV");
			_isPlaySound = true;
		}
		break;
	}

	_ani->position = position;
	_ani->Logic(elapsedTime);
}

void ClawDynamite::stopFalling(float collisionSize)
{
	if (_state == State::Explos) return;
	position.y -= collisionSize;
	speed = {};
	_ani->updateFrames = false;
	_state = State::Wait;
}
void ClawDynamite::stopMovingLeft(float collisionSize)
{
	if (_state == State::Explos) return;
	speed.x = -speed.x;
	position.x += collisionSize;
}
void ClawDynamite::stopMovingRight(float collisionSize)
{
	if (_state == State::Explos) return;
	speed.x = -speed.x;
	position.x -= collisionSize;
}
void ClawDynamite::bounceTop()
{
	if (_state == State::Explos) return;
	speed.y = abs(speed.y);
}
int ClawDynamite::getDamage() const
{
	if (_state == State::Explos)
		return ClawProjectile::getDamage();
	return 0;
}

bool ClawDynamite::isStartExplode() const
{
	return _state == State::Explos && _ani->getFrameNumber() == 1;
}
