#include "LordOmar.h"
#include "../ActionPlane.h"
#include "../Assets-Managers/AssetsManager.h"
#include "../Assets-Managers/PathManager.h"
#include "../Objects/EnemyProjectile.h"


/*
the order of the states:
  FireShield_1 - right down
  IceShield_1  - left down
  FireShield_2 - right down
  IceShield_2  - left down
  Bullet_1     - right up
  Bullet_2     - left up
  End          - end of the fight and get gem

Lord Omar is passive all the time, he doesn't move or attack.

*/


#define ANIMATION_IDLE			_animations["IDLE"]
#define ANIMATION_BLOCK			_animations["BLOCK"]
#define ANIMATION_THROW_ENERGY	_animations["STRIKE3"]


// TODO: save angles in radians instead of degrees
class LordOmarShield : public BaseDynamicPlaneObject
{
public:
	static const int NUM_OF_ITEMS_IN_FULL_SHIELD = 11;
	static const int NUM_OF_ITEMS_IN_SHIELD = 9;

	LordOmarShield(D2D1_POINT_2F center, const string& shieldItemImageSet, bool rotateClockwise)
		: BaseDynamicPlaneObject({}),
		_center(center),
		_rotateClockwise(rotateClockwise)
	{
		myMemCpy(ZCoord, DefaultZCoord::Characters + 1);
		_ani = AssetsManager::createAnimationFromDirectory(PathManager::getImageSetPath(shieldItemImageSet));

		// set the positions:
		const float step = 360.f / NUM_OF_ITEMS_IN_FULL_SHIELD;
		float angle = 0;
		for (int i = 0; i < NUM_OF_ITEMS_IN_SHIELD; i++)
		{
			_itemsAngles[i] = angle;
			angle += step;
		}

		ActionPlane::addPlaneObject(this);
	}

	void Logic(uint32_t elapsedTime) override
	{
		_ani->Logic(elapsedTime);

		const float rotateSpeed = 0.1;
		const float step = rotateSpeed * 360 / NUM_OF_ITEMS_IN_FULL_SHIELD;
		// rotate the shield: add `step` to each angle
		for (int i = 0; i < NUM_OF_ITEMS_IN_SHIELD; i++)
		{
			if (_rotateClockwise)
				_itemsAngles[i] += step;
			else
				_itemsAngles[i] -= step;

			if(_itemsAngles[i] > 360)
				_itemsAngles[i] -= 360;
			else if (_itemsAngles[i] < 0)
				_itemsAngles[i] += 360;
		}

		// block claw projectiles:
		for (Projectile* p : ActionPlane::getProjectiles())
		{
			if (isClawProjectile(p)) // actually, there are only ClawProjectiles in this state. TODO: delete this line ?
			{
				// if the projectile collides with shield item, remove it
				for (int i = 0; i < NUM_OF_ITEMS_IN_SHIELD; i++)
				{
					_ani->position.x = _center.x + 96 * cos(_itemsAngles[i] * M_PI / 180);
					_ani->position.y = _center.y + 96 * sin(_itemsAngles[i] * M_PI / 180);
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
		const int radius = 96;
		// draw the shield:
		for (int i = 0; i < NUM_OF_ITEMS_IN_SHIELD; i++)
		{
			_ani->position.x = _center.x + radius * cos(_itemsAngles[i] * M_PI / 180);
			_ani->position.y = _center.y + radius * sin(_itemsAngles[i] * M_PI / 180);
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
	const D2D1_POINT_2F _center;
	double _itemsAngles[NUM_OF_ITEMS_IN_SHIELD]; // save angle (in degrees) for each item of the shield
	const bool _rotateClockwise;
};

// TODO: when LO defeated should display the original animation (not "KILLFALL")


LordOmar::LordOmar(const WwdObject& obj)
	: BaseBoss(obj, 0, "", "HITHIGH", "HITLOW", "KILLFALL", "", "", ""), 
	_shield(nullptr), _state(States::FireShield_1), stateInited(false)
{
	_ani = ANIMATION_IDLE;
	_health = States::End; // get amount of states
}

void LordOmar::Logic(uint32_t elapsedTime)
{
	if (_ani->isFinishAnimation())
	{
		if (_ani == ANIMATION_BLOCK)
		{
			if (_state == States::Bullet_1 || _state == States::Bullet_2)
				_ani = ANIMATION_THROW_ENERGY;
			else
				_ani = ANIMATION_IDLE;
			_ani->reset();
		}
		else if (_ani == ANIMATION_THROW_ENERGY)
		{
			ActionPlane::addPlaneObject(DBG_NEW EnemyProjectile(
				AssetsManager::createAnimationFromDirectory(PathManager::getImageSetPath("LEVEL_OMARPROJECTILE")),
				20, { (_state == States::Bullet_1) ? -0.3f : 0.3f , 0 }, position));
			_ani->reset();
		}
	}

	// check for regualr attacks (sword, kick, etc.):
	if (checkForHurts())
	{
		if (_state == States::Bullet_1 || _state == States::Bullet_2)
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
		if (isClawProjectile(p))
		{
			ClawProjectile* cp = (ClawProjectile*)p;

			if (p->GetRect().intersects(GetRect()))
			{
				p->removeObject = true;

				// LO can hurt from projectiles only in these states
				if ((cp->type == ClawProjectile::Types::IceSword && (_state == States::FireShield_1 || _state == States::FireShield_2)) ||
					(cp->type == ClawProjectile::Types::FireSword && (_state == States::IceShield_1 || _state == States::IceShield_2)))
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
		case FireShield_1:
		case FireShield_2:
			position = { 42272, 2478 };
			_shield = DBG_NEW LordOmarShield(position, "LEVEL_FIRESHIELD", false);		
			_isMirrored = true;
			break;

		case IceShield_1:
		case IceShield_2:
			position = { 41376, 2478 };
			_shield = DBG_NEW LordOmarShield(position, "LEVEL_ICESHIELD", true);
			_isMirrored = false;
			break;

		case Bullet_1:
			position = { 42336, 1070 };
			_isMirrored = true;
			_ani = ANIMATION_THROW_ENERGY;
			_ani->reset();
			break;

		case Bullet_2:
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
	if (_state == States::Bullet_1 || _state == States::Bullet_2)
	{
		// CC can attack only in Bullet_1 and Bullet_2 states
		pair<Rectangle2D, int> clawAttackRect = player->GetAttackRect();
		if (clawAttackRect.second > 0 && clawAttackRect.first.intersects(_saveCurrRect))
		{
			return true;
		}
	}
	return false;
}

void LordOmar::advanceState()
{
	_state += 1;
	_health -= 1;
	stateInited = false;
	GetRect(); // update `_saveCurrRect`
	// TODO: cool efect of disappearing for Lord Omar and appear in the new position
}
