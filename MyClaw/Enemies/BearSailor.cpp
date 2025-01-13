#include "BearSailor.h"
#include "../GlobalObjects.h"

/*
STRIKE1 - hand up
STRIKE2 - send hand to catch CC
STRIKE3 - catch CC
STRIKE4 - I couldn't figure it out
*/


#define ANIMATION_STRIKE	_animations.at("STRIKE1")
#define ANIMATION_HUG		_animations.at("STRIKE3")


BearSailor::BearSailor(const WwdObject& obj)
	: BaseEnemy(obj, 14, 20, "FASTADVANCE", "HITHIGH",
		"HITLOW", "KILLFALL", "STRIKE1", "", "", "", "", ENEMY_PATROL_SPEED)
{
	// hug animation is too short, so we need to repeat it 3 times
	vector<UIAnimation::FrameData*> images;
	for (int i = 0; i < 3; i++)
		for (UIAnimation::FrameData* frame : ANIMATION_HUG->getImagesList())
			images.push_back(frame);
	ANIMATION_HUG = make_shared<UIAnimation>(images);

	drawZ = DefaultZCoord::Characters + 1; // when this enemy hug CC it should be above him
}

void BearSailor::Logic(uint32_t elapsedTime)
{
	const UIAnimation* prevAni = _ani.get();
	BaseEnemy::Logic(elapsedTime);
	if (prevAni == ANIMATION_HUG.get() && prevAni != _ani.get()) // hug animation is finished
		GO::unsqueezePlayer(); // CC is free now
}

void BearSailor::makeAttack(float deltaX, float deltaY)
{
	if (deltaY < 24)
	{
		if (deltaX < 64) // CC is close to enemy
		{
			_ani = ANIMATION_HUG;
			_ani->reset();
			_isStanding = false;
			_isAttack = true;
			_isMirrored = GO::getPlayerPosition().x < position.x;
			GO::squeezePlayer({ position.x + (_isMirrored ? -48 : 48), position.y - 8 }, !_isMirrored);
			_attackRest = 1000;
		}
		else if (deltaX < 96) // CC is little far from enemy
		{
			_ani = ANIMATION_STRIKE;
			_ani->reset();
			_isStanding = false;
			_isAttack = true;
			_isMirrored = GO::getPlayerPosition().x < position.x;
		}
	}
}

Rectangle2D BearSailor::GetRect()
{
	_saveCurrRect.left = position.x - 25;
	_saveCurrRect.right = position.x + 25;
	_saveCurrRect.top = position.y - 55;
	_saveCurrRect.bottom = position.y + 65;
	return _saveCurrRect;
}

pair<Rectangle2D, int> BearSailor::GetAttackRect()
{
	if (_ani->getFrameNumber() != 3) return {};

	Rectangle2D rc;

	if (_ani == ANIMATION_STRIKE)
	{
		if (_isMirrored)
		{
			rc.left = -60;
			rc.right = 20;
		}
		else
		{
			rc.left = 30;
			rc.right = 110;
		}

		rc.top = 25;
		rc.bottom = 80;
	}
	//else if (_ani == ANIMATION_HUG)
	//{
	//	// nop
	//}

	return { setRectByCenter(rc, _saveCurrRect), _damage };
}
