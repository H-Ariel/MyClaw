#include "LordOmarShield.h"
#include "../../../GlobalObjects.h"
#include "../../../Objects/Projectiles/ClawProjectile.h"


constexpr int NUM_OF_ITEMS_IN_FULL_SHIELD = 11;
constexpr int SHIELD_RADIUS = 96;
constexpr float M_2PI = float(2 * M_PI);
constexpr float STEP_SIZE = M_2PI / NUM_OF_ITEMS_IN_FULL_SHIELD; // the angle between each item of the shield (in radians)



LordOmarShield::LordOmarShield(D2D1_POINT_2F center, const string& shieldItemImageSet, bool rotateClockwise)
	: BaseDynamicPlaneObject({}),
	_center(center),
	_rotateClockwise(rotateClockwise)
{
	drawZ = DefaultZCoord::Characters + 1;
	_ani = AssetsManager::getAnimationFromDirectory(PathManager::getImageSetPath(shieldItemImageSet));

	// set the positions:

	float angle = 0;
	for (int i = 0; i < NUM_OF_ITEMS_IN_SHIELD; i++)
	{
		_itemsAngles[i] = angle;
		angle += STEP_SIZE;
	}

	GO::addObjectToActionPlane(this);
}

void LordOmarShield::Logic(uint32_t elapsedTime)
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
void LordOmarShield::Draw()
{
	for (int i = 0; i < NUM_OF_ITEMS_IN_SHIELD; i++)
	{
		_ani->position = getItemPosition(i);
		_ani->Draw();
	}
}
bool LordOmarShield::isFalling() const { return false; }
void LordOmarShield::stopFalling(float collisionSize) {}
void LordOmarShield::stopMovingLeft(float collisionSize) {}
void LordOmarShield::stopMovingRight(float collisionSize) {}
void LordOmarShield::bounceTop() {}
void LordOmarShield::whenTouchDeath() {}

D2D1_POINT_2F LordOmarShield::getItemPosition(int i) const
{
	return {
		_center.x + SHIELD_RADIUS * cosf(_itemsAngles[i]),
		_center.y + SHIELD_RADIUS * sinf(_itemsAngles[i])
	};
}
