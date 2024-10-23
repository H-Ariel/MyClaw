#include "EnemyProjectile.h"
#include "../ActionPlane.h"


EnemyProjectile::EnemyProjectile(const WwdObject& obj, const string& projectileAniDir)
	: Projectile(obj, PathManager::getImageSetPath(projectileAniDir)) {}
EnemyProjectile::EnemyProjectile(const WwdObject& obj, const string& projectileAni, const string& imageSet)
	: Projectile(obj, projectileAni, imageSet) {}
EnemyProjectile::EnemyProjectile(shared_ptr<UIAnimation> ani, int damage, D2D1_POINT_2F speed, D2D1_POINT_2F initialPosition)
	: Projectile(ani, damage, speed, initialPosition) {}

RatBomb::RatBomb(const WwdObject& obj)
	: EnemyProjectile(obj, PathManager::getAnimationPath("LEVEL_RATBOMB_FALLEASTWEST"))
{
	_isMirrored = !_isMirrored; // the rat-bomb is already mirrored
}
RatBomb::~RatBomb()
{
	if (removeObject)
	{
		actionPlane->addPlaneObject(DBG_NEW OneTimeAnimation(position, "LEVEL_RATBOMB_EXPLODE"));
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
		actionPlane->addPlaneObject(DBG_NEW OneTimeAnimation(position, "LEVEL_CRABBOMB_EXPLODE", "LEVEL_CRABBOMB"));
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
		actionPlane->addPlaneObject(DBG_NEW OneTimeAnimation(position, "LEVEL_GABRIELBOMB_EXPLODE", "LEVEL_GABRIELBOMB"));
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
		actionPlane->addPlaneObject(DBG_NEW OneTimeAnimation(position, "LEVEL_TRIDENTEXPLOSION", "LEVEL_TRIDENT_TRIDENTEXPLOSION"));
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

TProjectile::TProjectile(shared_ptr<UIAnimation> ani, int damage, D2D1_POINT_2F speed, D2D1_POINT_2F initialPosition)
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
		actionPlane->addPlaneObject(DBG_NEW OneTimeAnimation(position, "GAME_DYNAMITEEXPLO"));
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

LordOmarProjectile::LordOmarProjectile(D2D1_POINT_2F pos, float speedX)
	: EnemyProjectile(AssetsManager::createAnimationFromDirectory(PathManager::getImageSetPath("LEVEL_OMARPROJECTILE")), 20, { speedX , 0 }, pos)
{
}
Rectangle2D LordOmarProjectile::GetRect()
{
	return Rectangle2D(position.x - 16, position.y - 16, position.x + 16, position.y + 16);
}
