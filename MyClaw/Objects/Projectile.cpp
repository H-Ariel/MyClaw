#include "Projectile.h"
#include "../Assets-Managers/AssetsManager.h"
#include "ClawProjectile.h"
#include "EnemyProjectile.h"


Projectile::Projectile(const WwdObject& obj, const string& aniDirPath, const string& imageSet)
	: BaseDynamicPlaneObject(obj), _damage(obj.damage), _timeLeft(3000)
{
	if (aniDirPath.empty()) return;
	else if (endsWith(aniDirPath, ".ANI"))
		_ani = AssetsManager::loadCopyAnimation(aniDirPath, imageSet);
	else if (contains(aniDirPath, "SIREN"))
		_ani = AssetsManager::createCopyAnimationFromDirectory(aniDirPath);
	else
		_ani = AssetsManager::createAnimationFromDirectory(aniDirPath, false, 50);
	speed.x = obj.speedX / 1000.f;
	speed.y = obj.speedY / 1000.f;
	_isMirrored = speed.x < 0;
	myMemCpy(drawZ, DefaultZCoord::Characters + 1);
}
Projectile::Projectile(shared_ptr<Animation> ani, int damage, D2D1_POINT_2F speed, D2D1_POINT_2F initialPosition)
	: BaseDynamicPlaneObject({}), _damage(damage), _timeLeft(3000)
{
	_ani = ani;
	this->speed = speed;
	position = initialPosition;
	myMemCpy(drawZ, DefaultZCoord::Characters + 1);
}
void Projectile::Logic(uint32_t elapsedTime)
{
	if (!removeObject)
	{
		position.x += speed.x * elapsedTime;
		position.y += speed.y * elapsedTime;
		removeObject = speed.x == 0 && speed.y == 0;
	}
	if (_timeLeft > 0)
	{
		_timeLeft -= elapsedTime;
		if (_timeLeft <= 0)
			removeObject = true;
	}

	_ani->Logic(elapsedTime);
}
void Projectile::bounceTop() { speed.y = 0; }
void Projectile::stopFalling(float collisionSize) {}
bool Projectile::isClawBullet() const 
{
	if (isinstance<ClawProjectile>(this))
		return ((ClawProjectile*)this)->type == ClawProjectile::Types::Pistol;
	return false;
}
bool Projectile::isClawDynamite() const { return isinstance<ClawDynamite>(this); }
int Projectile::getDamage() const { return _damage; }


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
		typeid(GabrielBomb).hash_code(),
		typeid(CannonBall).hash_code(),
		typeid(MercatTrident).hash_code(),
		typeid(SirenProjectile).hash_code(),
		typeid(TProjectile).hash_code() // this is not from enemy, but they have same logic
	};
	return FindInArray(EnemiesProjectilesTypes, typeid(*obj).hash_code());
}
