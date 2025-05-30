#include "LordOmar.h"
#include "LordOmarShield.h"
#include "LordOmarProjectile.h"
#include "../../../GlobalObjects.h"
#include "../../../Objects/ClawProjectile.h"
#include "../../../Objects/OneTimeAnimation.h"


/*
Lord Omar is passive all the time, he doesn't move or attack.
The order of the states:
- Shield_1, Shield_3 - right down (fire shield)
- Shield_2, Shield_4 - left down  (ice shield)
- Bullet_1, Bullet_3 - right up   (energy ball)
- Bullet_2, Bullet_4 - left up    (energy ball)
- End - end of the fight and get gem

LO fade out before he moves to the next state.
*/


#define ANIMATION_IDLE			_animations["IDLE"]
#define ANIMATION_BLOCK			_animations["BLOCK"]
#define ANIMATION_THROW_ENERGY	_animations["STRIKE3"]

constexpr float FADEOUT_SPEED = 0.0025f;


LordOmar::LordOmar(const WwdObject& obj)
	: BaseBoss(obj), _shield(nullptr), _state(States::Shield_1), _stateInited(false), _fadeOut(false)
{
	_ani = ANIMATION_IDLE;
	_health = States::End; // the total amount of states
}
LordOmar::~LordOmar()
{
	if (removeObject)
	{
		_animations["KILLFALL3"]->mirrored = true;
		GO::addObjectToActionPlane(DBG_NEW OneTimeAnimation(position, _animations["KILLFALL3"], false));
	}
}
void LordOmar::Logic(uint32_t elapsedTime)
{
	if (_fadeOut)
	{
		if (_ani != _animations["HOME"])
		{
			_ani = _animations["HOME"];
			_ani->opacity = 1;
		}
		else if (_ani->isFinishAnimation())
		{
			_ani->opacity -= FADEOUT_SPEED * elapsedTime;
			if (_ani->opacity <= 0)
			{
				advanceState();
				_fadeOut = false;
			}
		}

		PostLogic(elapsedTime);
		return;
	}

	if (_stateInited)
	{
		if (_ani->isFinishAnimation())
		{
			if (_ani == ANIMATION_BLOCK)
			{
				if (States::Bullet_1 <= _state && _state <= States::Bullet_4)
					_ani = ANIMATION_THROW_ENERGY;
				else
					_ani = ANIMATION_IDLE;
				_ani->reset();
			}
			else if (_ani == ANIMATION_THROW_ENERGY)
			{
				GO::addObjectToActionPlane(DBG_NEW LordOmarProjectile({ position.x, position.y - 24 },
					(_state == States::Bullet_1 || _state == States::Bullet_3) ? -0.3f : 0.3f));
				_ani->reset();
			}
		}

		// check for regualr attacks (sword, kick, etc.):
		if (checkForHurts())
		{
			if (States::Bullet_1 <= _state && _state < States::Bullet_4)
			{
				_fadeOut = true;
			}
			else if (_state == States::Bullet_4)
			{
				advanceState();
			}
			else
			{
				_ani = ANIMATION_BLOCK;
				_ani->reset();
			}
		}

		// check for projectiles:
		for (Projectile* p : GO::getActionPlaneProjectiles())
		{
			if (isinstance<ClawProjectile>(p))
			{
				ClawProjectile* cp = (ClawProjectile*)p;

				if (p->GetRect().intersects(GetRect()))
				{
					p->removeObject = true;

					// LO can hurt from projectiles only in these states (ice projectiles break fire shield and vice versa)
					if ((cp->type == ClawProjectile::Types::IceSword && (_state == States::Shield_1 || _state == States::Shield_3)) ||
						(cp->type == ClawProjectile::Types::FireSword && (_state == States::Shield_2 || _state == States::Shield_4)))
					{
						_fadeOut = true;
						_shield->removeObject = true;
						_shield = nullptr; // the `ActionPlane` delete the object
					}
					else // for other cases, LO blocks the projectile
					{
						_ani = ANIMATION_BLOCK;
						_ani->reset();
					}
				}
			}
		}
	}
	else
	{
		shared_ptr<UIAnimation> shieldItem;
		switch (_state)
		{
		case Shield_1:
		case Shield_3:
			position = { 42272, 2478 };
			_shield = DBG_NEW LordOmarShield(position, "LEVEL_FIRESHIELD", false);
			_isMirrored = true;
			break;

		case Shield_2:
		case Shield_4:
			position = { 41376, 2478 };
			_shield = DBG_NEW LordOmarShield(position, "LEVEL_ICESHIELD", true);
			_isMirrored = false;
			break;

		case Bullet_1:
		case Bullet_3:
			position = { 42336, 1070 };
			_isMirrored = true;
			_ani = ANIMATION_THROW_ENERGY;
			_ani->reset();
			break;

		case Bullet_2:
		case Bullet_4:
			position = { 41312, 1070 };
			_isMirrored = false;
			_ani = ANIMATION_THROW_ENERGY;
			_ani->reset();
			break;

		case End:
			removeObject = true;
			break;
		}

		_ani->opacity = 1;

		_stateInited = true;
	}

	PostLogic(elapsedTime);
}
Rectangle2D LordOmar::GetRect()
{
	_saveCurrRect = Rectangle2D(
		position.x - 20,
		position.y - 50,
		position.x + 20,
		position.y + 60
	);
	return _saveCurrRect;
}
pair<Rectangle2D, int> LordOmar::GetAttackRect()
{
	return {};
}
bool LordOmar::checkForHurts()
{
	if (States::Bullet_1 <= _state && _state <= States::Bullet_4)
	{
		// CC can attack only in `Bullet_X` states
		pair<Rectangle2D, int> clawAttackRect = GO::getPlayerAttackRect();
		return (clawAttackRect.second > 0 && clawAttackRect.first.intersects(_saveCurrRect));
	}
	return false;
}
void LordOmar::advanceState()
{
	_state += 1;
	_health -= 1;
	_stateInited = false;
}
