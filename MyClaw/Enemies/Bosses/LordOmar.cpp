#include "LordOmar.h"
#include "../../GlobalObjects.h"
#include "../../Objects/EnemyProjectile.h"
#include "../../Objects/ClawProjectile.h"


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



constexpr int NUM_OF_ITEMS_IN_FULL_SHIELD = 11;
constexpr int NUM_OF_ITEMS_IN_SHIELD = 9;
constexpr int SHIELD_RADIUS = 96;
constexpr float M_2PI = float(2 * M_PI);
constexpr float STEP_SIZE = (M_2PI / NUM_OF_ITEMS_IN_FULL_SHIELD); // the angle between each item of the shield (in radians)

class LordOmarShield : public BaseDynamicPlaneObject
{
public:
	LordOmarShield(D2D1_POINT_2F center, const string& shieldItemImageSet, bool rotateClockwise)
		: BaseDynamicPlaneObject({}),
		_center(center),
		_rotateClockwise(rotateClockwise)
	{
		drawZ = DefaultZCoord::Characters + 1;
		_ani = AssetsManager::createAnimationFromDirectory(PathManager::getImageSetPath(shieldItemImageSet));

		// set the positions:
		
		float angle = 0;
		for (int i = 0; i < NUM_OF_ITEMS_IN_SHIELD; i++)
		{
			_itemsAngles[i] = angle;
			angle += STEP_SIZE;
		}

		GO::addObjectToActionPlane(this);
	}

	void Logic(uint32_t elapsedTime) override
	{
		_ani->Logic(elapsedTime);

		// to rotate the shield we add `rotatingStep` to each angle
		float rotatingStep = STEP_SIZE * elapsedTime / 128;
		for (int i = 0; i < NUM_OF_ITEMS_IN_SHIELD; i++)
		{
			if (_rotateClockwise)
				_itemsAngles[i] += rotatingStep;
			else
				_itemsAngles[i] -= rotatingStep;

			// normalize the angle to be in [0, 2*PI).
			// since `cos` and `sin` are periodic functions, I think that we don't need to normalize the angle
			if (_itemsAngles[i] >= M_2PI) _itemsAngles[i] -= M_2PI;
			else if (_itemsAngles[i] < 0) _itemsAngles[i] += M_2PI;
		}

		// block claw projectiles:
		for (Projectile* p : GO::getActionPlaneProjectiles())
		{
			if (isinstance<ClawProjectile>(p)) // actually, there are only ClawProjectiles in this state.
			{
				// if the projectile collides with shield item, remove it
				for (int i = 0; i < NUM_OF_ITEMS_IN_SHIELD; i++)
				{
					_ani->position = getItemPosition(i);
					if (p->GetRect().intersects(_ani->GetRect()))
					{
						p->removeObject = true;
						return;
					}
				}
			}
		}
	}
	void Draw() override
	{
		for (int i = 0; i < NUM_OF_ITEMS_IN_SHIELD; i++)
		{
			_ani->position = getItemPosition(i);
			_ani->Draw();
		}
	}
	bool isFalling() const override { return false; }
	void stopFalling(float collisionSize) override {}
	void stopMovingLeft(float collisionSize) override {}
	void stopMovingRight(float collisionSize) override {}
	void bounceTop() override {}
	void whenTouchDeath() override {}

private:
	D2D1_POINT_2F getItemPosition(int i) const
	{
		return {
			_center.x + SHIELD_RADIUS * cosf(_itemsAngles[i]),
			_center.y + SHIELD_RADIUS * sinf(_itemsAngles[i])
		};
	}

	const D2D1_POINT_2F _center;
	float _itemsAngles[NUM_OF_ITEMS_IN_SHIELD]; // save angle (in radians) for each item of the shield
	const bool _rotateClockwise;
};


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
