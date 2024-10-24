#include "Chameleon.h"
#include "../Objects/Player.h"


#define OPACITY_SPEED 0.005f


// it doesn't feel like the original game, but it's the best I can do for now

Chameleon::Chameleon(const WwdObject& obj)
	: BaseEnemy(obj, 1, 10, "FASTADVANCE", "HIT", "HIT", "KILLFALL",
		"STRIKE1", "", "", "", "", ENEMY_PATROL_SPEED), _opacity(1)
{
}

void Chameleon::Logic(uint32_t elapsedTime)
{
	BaseEnemy::Logic(elapsedTime);

	if (!_isAttack && doesEnemySeeClaw())
	{
		_opacity -= OPACITY_SPEED * elapsedTime;
		if (_opacity < 0)
			_opacity = 0;
	}
	else
	{
		_opacity += OPACITY_SPEED * elapsedTime;
		if (_opacity > 1)
			_opacity = 1;
	}

	_ani->opacity = _opacity;
}

Rectangle2D Chameleon::GetRect()
{
	_saveCurrRect.left = position.x - 20;
	_saveCurrRect.right = position.x + 20;
	_saveCurrRect.top = position.y - 25;
	_saveCurrRect.bottom = position.y + 35;
	return _saveCurrRect;
}

pair<Rectangle2D, int> Chameleon::GetAttackRect()
{
	Rectangle2D rc;

	rc.top = 10;
	rc.bottom = 30;

	if (_isMirrored)
	{
		rc.left = -90;
		rc.right = 10;
	}
	else
	{
		rc.left = 30;
		rc.right = 130;
	}

	return { setRectByCenter(rc, _saveCurrRect), _damage };
}

void Chameleon::makeAttack(float deltaX, float deltaY)
{
	if (_opacity != 1) return;

	if (deltaX < 128 && deltaY < 32) // CC is close to enemy
	{
		_ani = _animations["STRIKE1"];
		_ani->reset();
		_isStanding = false;
		_isAttack = true;
		_isMirrored = player->position.x < position.x;
	}
}
bool Chameleon::checkForHurts()
{
	return (_opacity == 1) ? BaseEnemy::checkForHurts() : false;
}
