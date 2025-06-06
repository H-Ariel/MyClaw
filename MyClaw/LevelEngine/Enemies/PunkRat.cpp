#include "PunkRat.h"
#include "../GlobalObjects.h"


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
	_cannon->_shootDirection = _cannon->_isMirrored ? Cannon::ToLeft : Cannon::ToRight;
	Rectangle2D cRc = _cannon->GetRect();
	myMemCpy(_minX, cRc.left);
	myMemCpy(_maxX, cRc.right);
}
PunkRat::~PunkRat()
{
	if (removeObject)
	{
		_cannon->_shootDirection = Cannon::NotShoot;
		GO::addObjectToActionPlane(_cannon);
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

	const UIAnimation* prevAni = _ani.get();

	position.x += speed.x * elapsedTime;
	if (position.x < _minX) { stopMovingLeft(_minX - position.x); _ani = PUNKRAT_STAND; }
	else if (position.x > _maxX) { stopMovingRight(position.x - _maxX); _ani = PUNKRAT_STAND; }

	if (_ani == PUNKRAT_STAND)
	{
		_ani = PUNKRAT_WALK;
	}

	if (_ani.get() != prevAni)
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
