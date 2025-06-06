#include "SirenScream.h"


SirenScream::SirenScream(const WwdObject& obj, int delay)
	: EnemyProjectile(obj, "LEVEL_SIRENPROJECTILE")//, _delay(delay)
{
	Delay(delay);
	_isMirrored = !_isMirrored; // the siren-projectile is already mirrored
}

void SirenScream::Logic(uint32_t elapsedTime)
{
	EnemyProjectile::Logic(elapsedTime);
	removeObject = _ani->isFinishAnimation();
}

void SirenScream::Draw()
{
	if (!isDelayed())
		_ani->Draw();
}
