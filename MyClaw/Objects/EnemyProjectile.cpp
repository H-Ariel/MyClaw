#include "EnemyProjectile.h"
#include "../ActionPlane.h"
#include "../Assets-Managers/AssetsManager.h"


EnemyProjectile::EnemyProjectile(const WwdObject& obj, const string& projectileAniDir, const string& imageSet)
	: Projectile(obj, PathManager::getImageSetPath(projectileAniDir)) {}
EnemyProjectile::EnemyProjectile(shared_ptr<Animation> ani, int damage, D2D1_POINT_2F speed, D2D1_POINT_2F initialPosition)
	: Projectile(ani, damage, speed, initialPosition) {} // TODO: apply for all enemies?

RatBomb::RatBomb(const WwdObject& obj)
	: EnemyProjectile(obj, PathManager::getAnimationPath("LEVEL_RATBOMB_FALLEASTWEST"))
{
	_isMirrored = !_isMirrored; // the rat-bomb is already mirrored
}
RatBomb::~RatBomb()
{
	if (removeObject)
	{
		ActionPlane::addPlaneObject(DBG_NEW OneTimeAnimation(position, "LEVEL_RATBOMB_EXPLODE"));
	}
}

CrabBomb::CrabBomb(const WwdObject& obj)
	: EnemyProjectile(obj, PathManager::getAnimationPath("LEVEL_CRABBOMB_FALL"),
		PathManager::getImageSetPath("LEVEL_CRABBOMB"))
{
}
CrabBomb::~CrabBomb()
{
	if (removeObject)
	{
		ActionPlane::addPlaneObject(DBG_NEW OneTimeAnimation(position, "LEVEL_CRABBOMB_EXPLODE", "LEVEL_CRABBOMB"));
	}
}

GabrielBomb::GabrielBomb(const WwdObject& obj)
	: EnemyProjectile(obj, PathManager::getAnimationPath("LEVEL_GABRIELBOMB_FALL"),
		PathManager::getImageSetPath("LEVEL_GABRIELBOMB"))
{
}
GabrielBomb::~GabrielBomb()
{
	if (removeObject)
	{
		ActionPlane::addPlaneObject(DBG_NEW OneTimeAnimation(position, "LEVEL_GABRIELBOMB_EXPLODE", "LEVEL_GABRIELBOMB"));
	}
}
void GabrielBomb::Logic(uint32_t elapsedTime)
{
	if (!removeObject)
		speed.y += GRAVITY * elapsedTime;
	Projectile::Logic(elapsedTime);
}
void GabrielBomb::stopFalling(float collisionSize)
{
	position.y -= collisionSize;
	position.y -= 32;
	removeObject = true;
}

CannonBall::CannonBall(const WwdObject& obj)
	: EnemyProjectile(obj, PathManager::getImageSetPath("LEVEL_CANNONBALL")) {}

MercatTrident::MercatTrident(const WwdObject& obj)
	: EnemyProjectile(obj, "LEVEL_TRIDENT_TRIDENTPROJECTILE")
{
	_isMirrored = !_isMirrored; // the mercat-trident is already mirrored
}
MercatTrident::~MercatTrident()
{
	if (removeObject)
	{
		ActionPlane::addPlaneObject(DBG_NEW OneTimeAnimation(position, "LEVEL_TRIDENTEXPLOSION", "LEVEL_TRIDENT_TRIDENTEXPLOSION"));
	}
}

SirenProjectile::SirenProjectile(const WwdObject& obj, int delay)
	: EnemyProjectile(obj, "LEVEL_SIRENPROJECTILE"), _delay(delay)
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
		removeObject = _ani->isFinishAnimation();
	}
}
void SirenProjectile::Draw()
{
	if (_delay <= 0)
		_ani->Draw();
}

TProjectile::TProjectile(shared_ptr<Animation> ani, int damage, D2D1_POINT_2F speed, D2D1_POINT_2F initialPosition)
	: EnemyProjectile(ani, damage, speed, initialPosition) {}

LavahandProjectile::LavahandProjectile(D2D1_POINT_2F initialPosition, bool mirrored)
	: EnemyProjectile(AssetsManager::loadAnimation(
		PathManager::getAnimationPath("LEVEL_LAVAHANDPROJECTILE"),
		PathManager::getImageSetPath("LEVEL_LAVAHANDPROJECTILE")),
		10, { mirrored ? 0.45f : -0.45f, -0.65f }, initialPosition)
{
}
LavahandProjectile::~LavahandProjectile()
{
	if (removeObject)
	{
		OneTimeAnimation* explosion = DBG_NEW OneTimeAnimation(position, "GAME_DYNAMITEEXPLO");
		myMemCpy(explosion->drawZ, DefaultZCoord::Characters + 1);
		ActionPlane::addPlaneObject(explosion);
	}
}
void LavahandProjectile::Logic(uint32_t elapsedTime)
{
	removeObject = speed.x == 0 && speed.y == 0;
	speed.y += GRAVITY * elapsedTime;
	position.x += speed.x * elapsedTime;
	position.y += speed.y * elapsedTime;
}
void LavahandProjectile::stopFalling(float collisionSize) 
{
	speed = {}; 
	position.y -= collisionSize; 
}
