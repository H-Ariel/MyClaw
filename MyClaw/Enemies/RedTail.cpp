#include "RedTail.h"
#include "../Assets-Managers/AssetsManager.h"
#include "../Player.h"
#include "../ActionPlane.h"


#define ANIMATION_BLOCK _animations["BLOCK"]
#define ANIMATION_SHOOT_HIGH _animations["STRIKE4"]


// TODO: write real logic for this boss (combine Red-Tail and wind)
RedTail::RedTail(const WwdObject& obj)
	: BaseBoss(obj, 10, "FASTADVANCE", "HITHIGH", "HITLOW", "KILLFALL", "STRIKE8",
		"STRIKE4", "LEVEL_REDTAILBULLET")
{
	_health = 100;
}

void RedTail::Logic(uint32_t elapsedTime)
{
	BaseEnemy::Logic(elapsedTime);
}

pair<Rectangle2D, int> RedTail::GetAttackRect()
{
	return pair<Rectangle2D, int>();
}

void RedTail::stopMovingLeft(float collisionSize)
{
	BaseBoss::stopMovingLeft(collisionSize);
}

void RedTail::stopMovingRight(float collisionSize)
{
	BaseBoss::stopMovingRight(collisionSize);
}

void RedTail::makeAttack(float deltaX, float deltaY)
{
	BaseBoss::makeAttack(deltaX, deltaY);
}

bool RedTail::checkForHurts()
{
	for (Projectile* p : ActionPlane::getProjectiles())
	{
		if (isbaseinstance<ClawProjectile>(p))
		{
			if (_saveCurrRect.intersects(p->GetRect()))
			{
				_ani = ANIMATION_BLOCK;
				_ani->reset();
				return false;
			}
		}
	}

	if (checkClawHit())
	{
		if (_blockClaw)
		{
			_ani = ANIMATION_BLOCK;
			return false;
		}
		else
		{
			// jump/block every 4 hits
			_hitsCuonter = (_hitsCuonter + 1) % 4;
			_canJump = true;
			return true;
		}
	}

	return false;
}



RedTailSpikes::RedTailSpikes(const WwdObject& obj)
	: BaseDamageObject(obj, 20)
{
	_ani = AssetsManager::createAnimationFromDirectory(PathManager::getImageSetPath(obj.imageSet));
	_ani->updateFrames = false; // only one frame, no need to update
	setObjectRectangle();
}
bool RedTailSpikes::isDamage() const { return true; }


// TODO: find when we activate the wind
RedTailWind::RedTailWind(const WwdObject& obj)
	: BasePlaneObject(obj), _windTimeCounter(0) {}
void RedTailWind::Logic(uint32_t elapsedTime)
{
	if (_windTimeCounter > 0)
	{
		_windTimeCounter -= elapsedTime;
		player->position.x -= 0.1f * elapsedTime;
	}
}
void RedTailWind::activate() { _windTimeCounter = 1000; }
