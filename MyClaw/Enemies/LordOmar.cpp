#include "LordOmar.h"
#include "../ActionPlane.h"
#include "../Assets-Managers/AssetsManager.h"
#include "../Assets-Managers/PathManager.h"


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


#define ANIMATION_IDLE	_animations["IDLE"]
#define ANIMATION_BLOCK	_animations["BLOCK"]


shared_ptr<Animation> FIRE_SHIELD_ITEM, ICE_SHIELD_ITEM, BULLET_ITEM;

// TODO: save angles in radians instead of degrees
class LordOmarShield : public BaseDynamicPlaneObject
{
public:
	static const int NUM_OF_ITEMS_IN_FULL_SHIELD = 11;
	static const int NUM_OF_ITEMS_IN_SHIELD = 9;

	LordOmarShield(D2D1_POINT_2F center, shared_ptr<Animation> shieldItem, bool rotateClockwise)
		: BaseDynamicPlaneObject({}),
		_center(center),
		_rotateClockwise(rotateClockwise)
	{
		myMemCpy(ZCoord, DefaultZCoord::Characters + 1);

		// set the positions:
		const float step = 360.f / NUM_OF_ITEMS_IN_FULL_SHIELD;
		float angle = 0;
		for (int i = 0; i < NUM_OF_ITEMS_IN_SHIELD; i++)
		{
			_itemsAngles[i] = angle;
			angle += step;
		}

		// set the animation:
		_ani = shieldItem;
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


LordOmarShield* fireShield = nullptr, * iceShield = nullptr;
bool hasShield = false;

LordOmar::LordOmar(const WwdObject& obj)
	: BaseBoss(obj, 0, "", "", "", "KILLFALL", "", "STRIKE3", ""), _state(States::FireShield_1)
{
	position.y += 10;
	_ani = ANIMATION_IDLE;
	_isMirrored = true;


//	FIRE_SHIELD_ITEM = AssetsManager::loadAnimation(PathManager::getAnimationPath("LEVEL_FIRESHIELD"), "LEVEL_FIRESHIELD");
//	ICE_SHIELD_ITEM = AssetsManager::loadAnimation(PathManager::getAnimationPath("LEVEL_ICESHIELD"), "LEVEL_ICESHIELD");

//	fireShield = DBG_NEW LordOmarShield(position, FIRE_SHIELD_ITEM);
//	iceShield = DBG_NEW LordOmarShield(position, ICE_SHIELD_ITEM);

//	ActionPlane::addPlaneObject(fireShield);
}

LordOmar::~LordOmar()
{
	fireShield = nullptr;
	iceShield = nullptr;
}

void LordOmar::Logic(uint32_t elapsedTime)
{
	if (!hasShield)
	{
		FIRE_SHIELD_ITEM = AssetsManager::createAnimationFromDirectory(PathManager::getImageSetPath("LEVEL_FIRESHIELD"));
		fireShield = DBG_NEW LordOmarShield(position, FIRE_SHIELD_ITEM, false);
		ActionPlane::addPlaneObject(fireShield);
		hasShield = true;
	}


	if (_ani == ANIMATION_BLOCK && _ani->isFinishAnimation())
	{
		_ani = ANIMATION_IDLE;
		_ani->reset();
	}


	for (Projectile* p : ActionPlane::getProjectiles())
	{
		if (isClawProjectile(p))
		{
			ClawProjectile* cp = (ClawProjectile*)p;

			if (p->GetRect().intersects(GetRect()))
			{
				p->removeObject = true;
				if (cp->type == ClawProjectile::Types::FireSword || cp->type == ClawProjectile::Types::IceSword)
				{
					// TODO: need to advance to next state
					// for now just end the fight
					this->removeObject = true;
				}
				else
				{
					_ani = ANIMATION_BLOCK;
					_ani->reset();
				}
			}
		}
	}


	PostLogic(elapsedTime);
}

Rectangle2D LordOmar::GetRect()
{
	return Rectangle2D(
		position.x - 20,
		position.y - 50,
		position.x + 20,
		position.y + 60
	);
}

pair<Rectangle2D, int> LordOmar::GetAttackRect()
{
	return {};
}

bool LordOmar::checkForHurts()
{
	return false;
}
