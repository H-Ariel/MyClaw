#include "ClawProjectile.h"
#include "../../GlobalObjects.h"


Projectile::Projectile(const WwdObject& obj, const string& aniDirPath, const string& imageSet)
	: BaseDynamicPlaneObject(obj), _damage(obj.damage)
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
	init();
}
Projectile::Projectile(shared_ptr<UIAnimation> ani, int damage, D2D1_POINT_2F speed, D2D1_POINT_2F initialPosition)
	: BaseDynamicPlaneObject({}), _damage(damage)
{
	_ani = ani;
	this->speed = speed;
	position = initialPosition;
	init();
}
Projectile::~Projectile()
{
	delete _timer;
}
void Projectile::init() {
	drawZ = DefaultZCoord::Characters + 1;
	_timer = DBG_NEW Timer(3000, [this] {removeObject = true; });
	GO::addTimer(_timer);
}

void Projectile::Logic(uint32_t elapsedTime)
{
	if (!removeObject)
	{
		position.x += speed.x * elapsedTime;
		position.y += speed.y * elapsedTime;
		removeObject = speed.x == 0 && speed.y == 0;
	}

	_ani->Logic(elapsedTime);
}
void Projectile::bounceTop() { speed.y = 0; }
void Projectile::stopFalling(float collisionSize) {}

// TODO `virtual... () const = 0`
bool Projectile::isClawBullet() const 
{
	if (isinstance<ClawProjectile>(this))
		return ((ClawProjectile*)this)->type == ClawProjectile::Types::Pistol;
	return false;
}
bool Projectile::isClawDynamite() const { return isinstance<ClawDynamite>(this); }
int Projectile::getDamage() const { return _damage; }
