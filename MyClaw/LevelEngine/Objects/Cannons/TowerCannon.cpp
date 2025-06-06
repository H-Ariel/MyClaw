#include "TowerCannon.h"


TowerCannon::TowerCannon(const WwdObject& obj)
	: Cannon(obj)
{
	if (endsWith(obj.logic, "Left")) _shootDirection = ToLeft;
	else if (endsWith(obj.logic, "Right")) _shootDirection = ToRight;
	else throw Exception('"' + obj.logic + "\" is not a tower-cannon");
	_ballOffset = 8;
}
