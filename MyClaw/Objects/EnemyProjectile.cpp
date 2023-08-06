#include "EnemyProjectile.h"
#include "../Assets-Managers/AssetsManager.h"
#include "../ActionPlane.h"


// TODO: use OneTimeAnimation
class BombExplos : public BaseStaticPlaneObject
{
public:
	BombExplos(const WwdObject& obj, const string& aniSet, const string& imageSet = "")
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


EnemyProjectile::EnemyProjectile(const WwdObject& obj, const string& projectileAniDir)
	: Projectile(obj, projectileAniDir) {} // TODO: use here the `PathManager::getImageSetPath` ...

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
		ActionPlane::addPlaneObject(DBG_NEW BombExplos(obj, "LEVEL_RATBOMB_EXPLODE"));
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
		ActionPlane::addPlaneObject(DBG_NEW BombExplos(obj, "LEVEL_CRABBOMB_EXPLODE", "LEVEL_CRABBOMB"));
	}
}

GabrielBomb::GabrielBomb(const WwdObject& obj)
	: Projectile(obj, PathManager::getAnimationPath("LEVEL_GABRIELBOMB_FALL"),
		PathManager::getImageSetPath("LEVEL_GABRIELBOMB"))
{
}
GabrielBomb::~GabrielBomb()
{
	if (removeObject)
	{
		WwdObject obj;
		obj.x = (int32_t)position.x;
		obj.y = (int32_t)position.y;
		obj.z = ZCoord;
		ActionPlane::addPlaneObject(DBG_NEW BombExplos(obj, "LEVEL_GABRIELBOMB_EXPLODE", "LEVEL_GABRIELBOMB"));
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
		ActionPlane::addPlaneObject(DBG_NEW BombExplos(obj,
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
