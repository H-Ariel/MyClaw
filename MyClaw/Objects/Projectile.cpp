#include "Projectile.h"
#include "../WindowManager.h"
#include "../AssetsManager.h"
#include "../ActionPlane.h"


Projectile::Projectile(const WwdObject& obj, int8_t damage, string aniDirPath)
	: BaseDynamicPlaneObject(obj), _damage(damage)
{
	if (endsWith(aniDirPath, ".ANI"))
	{
		_ani = AssetsManager::loadCopyAnimation(aniDirPath);
	}
	else
	{
		_ani = AssetsManager::createAnimationFromDirectory(aniDirPath, 75);
	}
	_speed.x = obj.speedX / 1000.f;
	_speed.y = obj.speedY / 1000.f;
	_isMirrored = _speed.x < 0;
}
void Projectile::Logic(uint32_t elapsedTime)
{
	if (!removeObject)
	{
		position.x += _speed.x * elapsedTime;
		removeObject = _speed.x == 0 && _speed.y == 0;
	}
}
void Projectile::bounceTop() { _speed.y = 0; }
void Projectile::stopFalling(float collisionSize) {}
bool Projectile::isClawBullet() const 
{
	if (isinstance<ClawProjectile>(this))
		return ((ClawProjectile*)this)->type == ClawProjectile::Types::Pistol;
	return false;
}
bool Projectile::isClawDynamite() const
{
	return isinstance<ClawDynamite>(this);
}
int8_t Projectile::getDamage() const { return _damage; }

ClawProjectile* ClawProjectile::createNew(Types type, const WwdObject& data)
{
	switch (type)
	{
	case Types::Pistol:		return DBG_NEW ClawProjectile(data, 8, "GAME/IMAGES/BULLETS", type);
	case Types::Magic:		return DBG_NEW ClawProjectile(data, 25, "GAME/IMAGES/MAGICCLAW", type);
	case Types::Dynamite:	return DBG_NEW ClawDynamite(data);
	case Types::FireSword:	return DBG_NEW ClawProjectile(data, 25, "GAME/IMAGES/PROJECTILES/FIRESWORD", type);
	case Types::IceSword:	return DBG_NEW ClawProjectile(data, 25, "GAME/IMAGES/PROJECTILES/ICESWORD", type);
	case Types::LightningSword:	return DBG_NEW ClawProjectile(data, 25, "GAME/IMAGES/PROJECTILES/LIGHTNINGSWORD", type);
	}
	return nullptr;
}
ClawProjectile::ClawProjectile(const WwdObject& obj, int8_t damage, string aniDirPath, Types type)
	: Projectile(obj, damage, aniDirPath), type(type) {}

ClawDynamite::ClawDynamite(const WwdObject& obj)
	: ClawProjectile(obj, 15, "GAME/ANIS/DYNAMITELIT.ANI", Types::Dynamite),
	_delayBeforeExplos(500), _state(State::Fly) {}
void ClawDynamite::Logic(uint32_t elapsedTime)
{
	switch (_state)
	{
	case State::Fly:
		position.x += elapsedTime * _speed.x;
		position.y += elapsedTime * _speed.y;
		_speed.y += elapsedTime * GRAVITY;
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
		break;
	}

	_ani->position = position;
	_ani->Logic(elapsedTime);
}
void ClawDynamite::stopFalling(float collisionSize)
{
	if (_state == State::Explos) return;
	position.y -= collisionSize;
	_speed = {};
	_ani->updateFrames = false;
	_state = State::Wait;
}
void ClawDynamite::stopMovingLeft(float collisionSize)
{
	if (_state == State::Explos) return;
	_speed.x = -_speed.x;
	position.x += collisionSize;
}
void ClawDynamite::stopMovingRight(float collisionSize)
{
	if (_state == State::Explos) return;
	_speed.x = -_speed.x;
	position.x -= collisionSize;
}
void ClawDynamite::bounceTop()
{
	if (_state == State::Explos) return;
	_speed.y = abs(_speed.y);
}
int8_t ClawDynamite::getDamage() const
{
	if (_state == State::Explos)
		return ClawProjectile::getDamage();
	return 0;
}

EnemyProjectile::EnemyProjectile(const WwdObject& obj, string projectileAniDir) : Projectile(obj, 10, projectileAniDir) {}

class RatBombExplos :public BaseStaticPlaneObject
{
public:
	RatBombExplos(const WwdObject& obj)
		: BaseStaticPlaneObject(obj)
	{
		_ani = AssetsManager::loadCopyAnimation(PathManager::getAnimationPath("LEVEL_RATBOMB_EXPLODE"));
		_ani->loopAni = false;
	}
	void Logic(uint32_t elapsedTime) override
	{
		_ani->Logic(elapsedTime);
		removeObject = _ani->isFinishAnimation();
	}
};
RatBomb::RatBomb(const WwdObject& obj)
	: Projectile(obj, 15, PathManager::getAnimationPath("LEVEL_RATBOMB_FALLEASTWEST"))
{
	_isMirrored = !_isMirrored; // the rat-bomb is already mirrored
}
RatBomb::~RatBomb()
{
	if (removeObject)
	{
		WwdObject obj;
		obj.x = (int32_t)position.x;
		obj.y = (int32_t)position.y;
		obj.z = ZCoord;
		ActionPlane::addPlaneObject(DBG_NEW RatBombExplos(obj));
	}
}

CannonBall::CannonBall(const WwdObject& obj) : Projectile(obj, 15, PathManager::getImageSetPath("LEVEL_CANNONBALL")) {}


bool isProjectile(BasePlaneObject* obj)
{
	return isbaseinstance<Projectile>(obj);
}
bool isClawProjectile(BasePlaneObject* obj)
{
	return isbaseinstance<ClawProjectile>(obj);
}
bool isEnemyProjectile(BasePlaneObject* obj)
{
	static initializer_list<size_t> EnemiesProjectilesTypes = {
		typeid(EnemyProjectile).hash_code(),
		typeid(RatBomb).hash_code(),
		typeid(CannonBall).hash_code()
	};
	return FindInArray(EnemiesProjectilesTypes, typeid(*obj).hash_code());
}
