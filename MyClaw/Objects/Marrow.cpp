#include "Marrow.h"
#include "../Player.h"


Marrow::Marrow(const WwdObject& obj, Player* player)
	: BaseBoss(obj, player, 10, "FASTADVANCE", "HITHIGH", "HITLOW",
		"KILLFALL", "STRIKE1", "STRIKE2", "GAME/IMAGES/BULLETS")
{
	_ani = _animations["HOME"];
	_forward = false;
}

void Marrow::Logic(uint32_t elapsedTime)
{
	if (!PreLogic(elapsedTime)) return;
	PostLogic(elapsedTime);
}

pair<Rectangle2D, int8_t> Marrow::GetAttackRect()
{
	return {};
}

bool Marrow::checkForHurts()
{
	return false;
}






#define MARROW_PARROT_SPEED 0.3f

MarrowParrot::MarrowParrot(const WwdObject& obj, Player* player)
	: BaseEnemy(obj, player, 3, 10, "FLY", "HIT", "HIT", "KILLFALL", "STRIKE1", "",
		"", "", "", MARROW_PARROT_SPEED, true),
	_flyRect(obj.minX, obj.minY, obj.maxX, obj.maxY)
{
//	_ani = _animations["FLY"];
	_forward = false;

	_speed = { 0, MARROW_PARROT_SPEED };
}

void MarrowParrot::Logic(uint32_t elapsedTime)
{
	if (!PreLogic(elapsedTime)) return;

	position.x += _speed.x * elapsedTime;
	position.y += _speed.y * elapsedTime;


	if (position.y > _flyRect.bottom)
	{
		position.y = _flyRect.bottom;
		_speed = { -MARROW_PARROT_SPEED, 0 };
	}

	if (position.x < _flyRect.left)
	{
		position.x = _flyRect.left;
		_speed = { 0, MARROW_PARROT_SPEED };
	}

	if (position.y < _flyRect.top)
	{
		position.y = _flyRect.top;
		_speed = { MARROW_PARROT_SPEED, 0 };
	}

	if (position.x > _flyRect.right)
	{
		position.x = _flyRect.right;
		_speed = { 0, -MARROW_PARROT_SPEED };
	}


	
	
	
	
	

	_forward = _speed.x > 0;

	PostLogic(elapsedTime);
}

pair<Rectangle2D, int8_t> MarrowParrot::GetAttackRect()
{
	return {};
}


void MarrowParrot::stopFalling(float collisionSize) {}
void MarrowParrot::stopMovingLeft(float collisionSize) {}
void MarrowParrot::stopMovingRight(float collisionSize) {}
void MarrowParrot::bounceTop() {}
