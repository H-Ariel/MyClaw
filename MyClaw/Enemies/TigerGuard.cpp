#include "TigerGuard.h"

// TODO (?): add ability to block CC attacks
// TODO (?): add ability to flip (escape from CC attacks)
// TODO: make TigerGuard harder to defeat

TigerGuard::TigerGuard(const WwdObject& obj)
	: BaseEnemy(obj, 9, 10, "FASTADVANCE", "HITHIGH", "HITLOW",
		"KILLFALL", "STRIKE1", "STRIKE2", "", "", "", ENEMY_PATROL_SPEED)
{
}

Rectangle2D TigerGuard::GetRect()
{
	Rectangle2D rc(
		position.x - 20, position.y - 50,
		position.x + 20, position.y + 60
	);
	_saveCurrRect = rc;
	return rc;
}

pair<Rectangle2D, int> TigerGuard::GetAttackRect()
{
	if (!_isAttack) return {};

	Rectangle2D rc;

	if (_isMirrored)
	{
		rc.left = -40;
		rc.right = 20;
	}
	else
	{
		rc.left = 30;
		rc.right = 90;
	}

	if (_ani == _animations["STRIKE1"]) // strike high
	{
		rc.top = 20;
		rc.bottom = 40;
	}
	else
	{
		rc.top = 80;
		rc.bottom = 100;
	}

	return { setRectByCenter(rc, _saveCurrRect), _damage };
}
