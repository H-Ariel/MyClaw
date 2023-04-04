#include "Rat.h"
#include "Projectile.h"
#include "../Player.h"
#include "../ActionPlane.h"


#define ANIMATION_WALK		_animations.at(_walkAniName)
#define ANIMATION_SHOOT		_animations.at(_shootAniName)


// the rat has its own Logic so we don't need pass all arguments
Rat::Rat(const WwdObject& obj, Player* player)
	: BaseEnemy(obj, player, 1, 0, "WALK", "DEAD", "DEAD",
		"DEAD", "", "", "THROWEASTWEST", "", "", true)
{
}

void Rat::makeAttack()
{
	const float deltaX = abs(_player->position.x - position.x);
	const float deltaY = abs(_player->position.y - position.y);
	const bool isInRange = (_forward && _player->position.x > position.x) || (!_forward && _player->position.x < position.x);

	if (_isStanding || isInRange)
	{
		if (deltaX < 352 && deltaY < 42)
		{
			_ani = ANIMATION_SHOOT;
			_ani->reset();
			_isStanding = false;
			_isAttack = true;
			_forward = _player->position.x > position.x;

			WwdObject obj;
			obj.x = (int32_t)(position.x + (_forward ? _saveCurrRect.right - _saveCurrRect.left : _saveCurrRect.left - _saveCurrRect.right));
			obj.y = (int32_t)position.y - 24;
			obj.z = ZCoord;
			obj.speedX = _forward ? DEFAULT_PROJECTILE_SPEED : -DEFAULT_PROJECTILE_SPEED;
			obj.damage = 15;
			ActionPlane::addPlaneObject(DBG_NEW RatBomb(obj));
		}
	}
}

pair<D2D1_RECT_F, int8_t> Rat::GetAttackRect() { return {}; }
bool Rat::isDuck() const { return true; }
bool Rat::isTakeDamage() const { return false; }



#define PUNKRAT_HOME	_animations.at("HOME")
#define PUNKRAT_IDLE	_animations.at("IDLE")
#define PUNKRAT_RECOIL	_animations.at("RECOIL")
#define PUNKRAT_STAND	_animations.at("STAND")
#define PUNKRAT_STRIKE	_animations.at("STRIKE")
#define PUNKRAT_WALK	_animations.at("WALK")
#define PUNKRAT_SPEED	0.07f

//TODO: use all animation
PunkRat::PunkRat(const WwdObject& obj, Player* player)
	: BaseEnemy(obj, player, 1, 0, "WALK", "IDLE",
		"IDLE", "IDLE", "", "", "", "", "", true)
{
	WwdObject cannonData(obj);
	cannonData.imageSet = "LEVEL_CANNON";
	_cannon = DBG_NEW Cannon(cannonData, player);
	_cannon->position.y += 20;
	_cannon->_ballOffset = 12;
	if (_isMirrored) _cannon->_shootDirection = Cannon::ToLeft;
	else _cannon->_shootDirection = Cannon::ToRight;

	D2D1_RECT_F cRc = _cannon->GetRect();
	myMemCpy(_minX, cRc.left);
	myMemCpy(_maxX, cRc.right);

	_speed.x = PUNKRAT_SPEED;
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

	position.x += _speed.x * elapsedTime;
	if (position.x < _minX) { stopMovingLeft(_minX - position.x); _ani = PUNKRAT_STAND; _speed.x = PUNKRAT_SPEED; }
	else if (position.x > _maxX) { stopMovingRight(position.x - _maxX); _ani = PUNKRAT_STAND; _speed.x = -PUNKRAT_SPEED; }

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
	BaseCharacter::Draw();
}
D2D1_RECT_F PunkRat::GetRect()
{
//	return _ani->GetRect();
	return BasePlaneObject::GetRect();
}
pair<D2D1_RECT_F, int8_t> PunkRat::GetAttackRect() { return{}; }
