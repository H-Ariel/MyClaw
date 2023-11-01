#include "Rat.h"
#include "../Player.h"
#include "../ActionPlane.h"
#include "../Objects/EnemyProjectile.h"


Rat::Rat(const WwdObject& obj)
	: BaseEnemy(obj, 1, 0, "WALK", "DEAD", "DEAD", "DEAD", "", "", "THROWEASTWEST", "", "", ENEMY_PATROL_SPEED, true)
{
}
void Rat::makeAttack(float deltaX, float deltaY)
{
	if (deltaX < 352 && deltaY < 42)
	{
		_ani = _animations[_shootAniName];
		_ani->reset();
		_isStanding = false;
		_isAttack = true;
		_isMirrored = player->position.x < position.x;

		WwdObject obj;
		obj.x = (int32_t)(position.x + (!_isMirrored ? _saveCurrRect.right - _saveCurrRect.left : _saveCurrRect.left - _saveCurrRect.right));
		obj.y = (int32_t)position.y - 32;
		obj.z = ZCoord;
		obj.speedX = !_isMirrored ? DEFAULT_PROJECTILE_SPEED : -DEFAULT_PROJECTILE_SPEED;
		obj.damage = 15;
		ActionPlane::addPlaneObject(DBG_NEW RatBomb(obj));
	}
}
pair<Rectangle2D, int> Rat::GetAttackRect() { return {}; }
bool Rat::isDuck() const { return true; }
bool Rat::isTakeDamage() const { return false; }


//#define PUNKRAT_HOME	_animations.at("HOME")
//#define PUNKRAT_IDLE	_animations.at("IDLE")
//#define PUNKRAT_RECOIL	_animations.at("RECOIL")
//#define PUNKRAT_STRIKE	_animations.at("STRIKE")
#define PUNKRAT_STAND	_animations.at("STAND")
#define PUNKRAT_WALK	_animations.at("WALK")


PunkRat::PunkRat(const WwdObject& obj)
	: BaseEnemy(obj, 1, 0, "WALK", "IDLE",
		"IDLE", "IDLE", "", "", "", "", "", 0.07f, true)
{
	WwdObject cannonData(obj);
	cannonData.imageSet = "LEVEL_CANNON";
	_cannon = DBG_NEW Cannon(cannonData);
	_cannon->position.y += 20;
	_cannon->_ballOffset = 12;
	if (_cannon->_isMirrored) _cannon->_shootDirection = Cannon::ToLeft;
	else _cannon->_shootDirection = Cannon::ToRight;
	Rectangle2D cRc = _cannon->GetRect();
	myMemCpy(_minX, cRc.left);
	myMemCpy(_maxX, cRc.right);
}
PunkRat::~PunkRat()
{
	if (removeObject)
	{
		_cannon->_shootDirection = Cannon::NotShoot;
		ActionPlane::addPlaneObject(_cannon);
	}
	else
	{
		// the rat is alive and the cannon does not pass to `ActionPlane`
		delete _cannon;
	}
}
void PunkRat::Logic(uint32_t elapsedTime)
{
	_saveCurrRect = _ani->GetRect();
	if (checkForHurts())
		return;

	const shared_ptr<Animation> prevAni = _ani;

	position.x += speed.x * elapsedTime;
	if (position.x < _minX) { stopMovingLeft(_minX - position.x); _ani = PUNKRAT_STAND; }
	else if (position.x > _maxX) { stopMovingRight(position.x - _maxX); _ani = PUNKRAT_STAND; }

	if (_ani == PUNKRAT_STAND)
	{
		_ani = PUNKRAT_WALK;
	}

	if (_ani != prevAni)
	{
		_ani->position = position;
		_ani->reset();
	}

	_ani->Logic(elapsedTime);
	_cannon->Logic(elapsedTime);
}
void PunkRat::Draw()
{
	_cannon->Draw();
	BaseEnemy::Draw();
}
Rectangle2D PunkRat::GetRect()
{
	return BasePlaneObject::GetRect();
}
pair<Rectangle2D, int> PunkRat::GetAttackRect() { return{}; }
