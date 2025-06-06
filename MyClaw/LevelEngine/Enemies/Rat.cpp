#include "Rat.h"
#include "../GlobalObjects.h"
#include "../Objects/Projectiles/RatBomb.h"


Rat::Rat(const WwdObject& obj)
	: BaseEnemy(obj, 1, 0, "WALK", "DEAD", "DEAD", "DEAD", "", "", "THROWEASTWEST", "", "", ENEMY_PATROL_SPEED, true)
{
}
void Rat::makeAttack(float deltaX, float deltaY)
{
	if (deltaX < 352 && deltaY < 42)
	{
		_ani = _aniShoot;
		_ani->reset();
		_isStanding = false;
		_isAttack = true;
		_isMirrored = GO::getPlayerPosition().x < position.x;

		WwdObject obj;
		obj.x = (int32_t)(position.x + (!_isMirrored ? _saveCurrRect.right - _saveCurrRect.left : _saveCurrRect.left - _saveCurrRect.right));
		obj.y = (int32_t)position.y - 32;
		obj.speedX = !_isMirrored ? DEFAULT_PROJECTILE_SPEED : -DEFAULT_PROJECTILE_SPEED;
		obj.damage = 15;
		GO::addObjectToActionPlane(DBG_NEW RatBomb(obj));
	}
}
pair<Rectangle2D, int> Rat::GetAttackRect() { return {}; }
bool Rat::isDuck() const { return true; }
bool Rat::isTakeDamage() const { return false; }
