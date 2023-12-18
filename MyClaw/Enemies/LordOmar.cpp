#include "LordOmar.h"
#include "../ActionPlane.h"
#include "../Assets-Managers/AssetsManager.h"
#include "../Assets-Managers/PathManager.h"
#include "../Objects/EnemyProjectile.h"


/*
Lord Omar is passive all the time, he doesn't move or attack.
The order of the states:
- Shield_1, Shield_3 - right down (fire shield)
- Shield_2, Shield_4 - left down  (ice shield)
- Bullet_1, Bullet_3 - right up   (energy ball)
- Bullet_2, Bullet_4 - left up    (energy ball)
- End - end of the fight and get gem
*/



#define NUM_OF_ITEMS_IN_FULL_SHIELD	11
#define NUM_OF_ITEMS_IN_SHIELD		9
#define SHIELD_RADIUS	96
#define M_2PI			float(2 * M_PI)
#define STEP_SIZE		(M_2PI / NUM_OF_ITEMS_IN_FULL_SHIELD) // the angle between each item of the shield (in radians)

class LordOmarShield : public BaseDynamicPlaneObject
{
public:
	LordOmarShield(D2D1_POINT_2F center, const string& shieldItemImageSet, bool rotateClockwise)
		: BaseDynamicPlaneObject({}),
		_center(center),
		_rotateClockwise(rotateClockwise)
	{
		myMemCpy(drawZ, DefaultZCoord::Characters + 1);
		_ani = AssetsManager::createAnimationFromDirectory(PathManager::getImageSetPath(shieldItemImageSet));

		// set the positions:
		
		float angle = 0;
		for (int i = 0; i < NUM_OF_ITEMS_IN_SHIELD; i++)
		{
			_itemsAngles[i] = angle;
			angle += STEP_SIZE;
		}

		ActionPlane::addPlaneObject(this);
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
		for (Projectile* p : ActionPlane::getProjectiles())
		{
			if (isbaseinstance<ClawProjectile>(p)) // actually, there are only ClawProjectiles in this state.
			{
				// if the projectile collides with shield item, remove it
				for (int i = 0; i < NUM_OF_ITEMS_IN_SHIELD; i++)
				{
					_ani->position = getItemPosition(i);
					_ani->updateImageData();
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
			_ani->updateImageData();
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


LordOmar::LordOmar(const WwdObject& obj)
	: BaseBoss(obj), _shield(nullptr), _state(States::Shield_1), stateInited(false)
{
	_ani = ANIMATION_IDLE;
	_health = States::End; // the total amount of states
}
LordOmar::~LordOmar()
{
	if (removeObject)
	{
		_animations["KILLFALL3"]->mirrored = true;
		ActionPlane::addPlaneObject(DBG_NEW OneTimeAnimation(position, _animations["KILLFALL3"], false));
	}
}
void LordOmar::Logic(uint32_t elapsedTime)
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
			D2D1_POINT_2F projPos = position;
			projPos.y -= 16;
			ActionPlane::addPlaneObject(DBG_NEW EnemyProjectile(
				AssetsManager::createAnimationFromDirectory(PathManager::getImageSetPath("LEVEL_OMARPROJECTILE")),
				20, { (_state == States::Bullet_1 || _state == States::Bullet_3) ? -0.3f : 0.3f , 0 }, projPos));
			_ani->reset();
		}
	}

	// check for regualr attacks (sword, kick, etc.):
	if (checkForHurts())
	{
		if (States::Bullet_1 <= _state && _state <= States::Bullet_4)
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
	for (Projectile* p : ActionPlane::getProjectiles())
	{
		if (isbaseinstance<ClawProjectile>(p))
		{
			ClawProjectile* cp = (ClawProjectile*)p;

			if (p->GetRect().intersects(GetRect()))
			{
				p->removeObject = true;

				// LO can hurt from projectiles only in these states (ice projectiles break fire shield and vice versa)
				if ((cp->type == ClawProjectile::Types::IceSword && (_state == States::Shield_1 || _state == States::Shield_3)) ||
					(cp->type == ClawProjectile::Types::FireSword && (_state == States::Shield_2 || _state == States::Shield_4)))
				{
					advanceState();
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

	if (!stateInited)
	{
		shared_ptr<Animation> shieldItem;
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

		stateInited = true;
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
		pair<Rectangle2D, int> clawAttackRect = player->GetAttackRect();
		return (clawAttackRect.second > 0 && clawAttackRect.first.intersects(_saveCurrRect));
	}
	return false;
}
void LordOmar::advanceState()
{
	_state += 1;
	_health -= 1;
	stateInited = false;
	// TODO: cool efect of disappearing for Lord Omar and appear in the new position
}
