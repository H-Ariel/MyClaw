#include "Projectile.h"
#include "../WindowManager.h"
#include "../AssetsManager.h"
#include "../ActionPlane.h"


Projectile::Projectile(const WwdObject& obj, const string& aniDirPath, const string& imageSet)
	: BaseDynamicPlaneObject(obj), _damage(obj.damage), _timeLeft(3000)
{
	if (endsWith(aniDirPath, ".ANI"))
		_ani = AssetsManager::loadCopyAnimation(aniDirPath, imageSet);
	else if (contains(aniDirPath, "SIREN"))
		_ani = AssetsManager::createCopyAnimationFromDirectory(aniDirPath, 100, false);
	else
		_ani = AssetsManager::createAnimationFromDirectory(aniDirPath, 75, false);
	_speed.x = obj.speedX / 1000.f;
	_speed.y = obj.speedY / 1000.f;
	_isMirrored = _speed.x < 0;
}
Projectile::Projectile(shared_ptr<Animation> ani, int8_t damage, D2D1_POINT_2F speed, D2D1_POINT_2F initialPosition)
	: BaseDynamicPlaneObject({}), _damage(damage), _timeLeft(3000)
{
	_ani = ani;
	_speed = speed;
	position = initialPosition;
}
void Projectile::Logic(uint32_t elapsedTime)
{
	if (!removeObject)
	{
		position.x += _speed.x * elapsedTime;
		position.y += _speed.y * elapsedTime;
		removeObject = _speed.x == 0 && _speed.y == 0;
	}
	if (_timeLeft > 0)
	{
		_timeLeft -= elapsedTime;
		if (_timeLeft <= 0)
			removeObject = true;
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
int8_t Projectile::getDamage() const { return _damage; }

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

ClawDynamite::ClawDynamite(const WwdObject& obj)
	: ClawProjectile(obj, "GAME/ANIS/DYNAMITELIT.ANI", Types::Dynamite),
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

EnemyProjectile::EnemyProjectile(const WwdObject& obj, const string& projectileAniDir)
	: Projectile(obj, projectileAniDir) {} // TODO: use here the `PathManager::getImageSetPath` ...

class RatBombExplos : public BaseStaticPlaneObject
{
public:
	RatBombExplos(const WwdObject& obj, const string& aniSet, const string& imageSet = "")
		: BaseStaticPlaneObject(obj)
	{
		_ani = AssetsManager::loadCopyAnimation(PathManager::getAnimationPath(aniSet),
			imageSet.empty() ? "" : PathManager::getImageSetPath(imageSet));
		_ani->loopAni = false;
	}
	void Logic(uint32_t elapsedTime) override
	{
		_ani->Logic(elapsedTime);
		removeObject = _ani->isFinishAnimation();
	}
};
RatBomb::RatBomb(const WwdObject& obj)
	: Projectile(obj, PathManager::getAnimationPath("LEVEL_RATBOMB_FALLEASTWEST"))
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
		ActionPlane::addPlaneObject(DBG_NEW RatBombExplos(obj, "LEVEL_RATBOMB_EXPLODE"));
	}
}

CrabBomb::CrabBomb(const WwdObject& obj)
	: Projectile(obj, PathManager::getAnimationPath("LEVEL_CRABBOMB_FALL"),
		PathManager::getImageSetPath("LEVEL_CRABBOMB"))
{
}
CrabBomb::~CrabBomb()
{
	if (removeObject)
	{
		WwdObject obj;
		obj.x = (int32_t)position.x;
		obj.y = (int32_t)position.y;
		obj.z = ZCoord;
		ActionPlane::addPlaneObject(DBG_NEW RatBombExplos(obj, "LEVEL_CRABBOMB_EXPLODE", "LEVEL_CRABBOMB"));
	}
}


CannonBall::CannonBall(const WwdObject& obj)
	: Projectile(obj, PathManager::getImageSetPath("LEVEL_CANNONBALL")) {}

MercatTrident::MercatTrident(const WwdObject& obj)
	: EnemyProjectile(obj, PathManager::getImageSetPath("LEVEL_TRIDENT_TRIDENTPROJECTILE"))
{
	_isMirrored = !_isMirrored; // the mercat-trident is already mirrored
}
MercatTrident::~MercatTrident()
{
	if (removeObject)
	{
		WwdObject obj;
		obj.x = (int32_t)position.x;
		obj.y = (int32_t)position.y;
		obj.z = ZCoord;
		ActionPlane::addPlaneObject(DBG_NEW RatBombExplos(obj,
			"LEVEL_TRIDENTEXPLOSION", "LEVEL_TRIDENT_TRIDENTEXPLOSION"));
	}
}

SirenProjectile::SirenProjectile(const WwdObject& obj, int32_t delay)
	: EnemyProjectile(obj, PathManager::getImageSetPath("LEVEL_SIRENPROJECTILE")), _delay(delay)
{
	_isMirrored = !_isMirrored; // the siren-projectile is already mirrored
}
void SirenProjectile::Logic(uint32_t elapsedTime)
{
	if (_delay > 0)
	{
		_delay -= elapsedTime;
	}
	else
	{
		EnemyProjectile::Logic(elapsedTime);
		_ani->Logic(elapsedTime);
		removeObject = _ani->isFinishAnimation();
	}
}
void SirenProjectile::Draw()
{
	if (_delay <= 0)
		_ani->Draw();
}

TProjectile::TProjectile(shared_ptr<Animation> ani, int8_t damage, D2D1_POINT_2F speed, D2D1_POINT_2F initialPosition)
	: Projectile(ani, damage, speed, initialPosition) {}

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
		typeid(CrabBomb).hash_code(),
		typeid(CannonBall).hash_code(),
		typeid(MercatTrident).hash_code(),
		typeid(SirenProjectile).hash_code(),
		typeid(TProjectile).hash_code() // this is not from enemy, but they have same logic
	};
	return FindInArray(EnemiesProjectilesTypes, typeid(*obj).hash_code());
}
