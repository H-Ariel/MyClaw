#include "MercatTridentProjectile.h"
#include "../OneTimeAnimation.h"
#include "../../GlobalObjects.h"


MercatTridentProjectile::MercatTridentProjectile(const WwdObject& obj)
	: EnemyProjectile(obj, "LEVEL_TRIDENT_TRIDENTPROJECTILE")
{
	_isMirrored = !_isMirrored; // the mercat-trident is already mirrored
}
MercatTridentProjectile::~MercatTridentProjectile()
{
	if (removeObject)
	{
		GO::addObjectToActionPlane(DBG_NEW OneTimeAnimation(position, "LEVEL_TRIDENTEXPLOSION", "LEVEL_TRIDENT_TRIDENTEXPLOSION"));
	}
}
