#include "SkullCannon.h"


SkullCannon::SkullCannon(const WwdObject& obj)
	: Cannon(obj)
{
	_shootDirection = _isMirrored ? ToLeft : ToRight;
	_ballOffset = 8;
}
