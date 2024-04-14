#include "Projectile.h"
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
	drawZ = DefaultZCoord::Characters + 1;
}
Projectile::Projectile(shared_ptr<UIAnimation> ani, int damage, D2D1_POINT_2F speed, D2D1_POINT_2F initialPosition)
	: BaseDynamicPlaneObject({}), _damage(damage), _timeLeft(3000)
{
	_ani = ani;
	this->speed = speed;
	position = initialPosition;
	drawZ = DefaultZCoord::Characters + 1;
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
