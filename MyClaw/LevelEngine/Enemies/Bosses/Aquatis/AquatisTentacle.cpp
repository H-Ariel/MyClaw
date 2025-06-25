#include "Aquatis.h"
#include "AquatisTentacle.h"
#include "../../../GlobalObjects.h"
#include "../../../Objects/OneTimeAnimation.h"
#include "../../../Objects/Projectiles/ClawProjectile.h"


AquatisTentacle::AquatisTentacle(const WwdObject& obj)
	: BaseDamageObject(obj, 5), _squeezeRestTime(0), _deadTime(0)
{
	string imageSet = PathManager::getImageSetPath(obj.imageSet);
	string aniSet = PathManager::getAnimationSetPath(obj.imageSet);

	_idle = AssetsManager::loadCopyAnimation(aniSet + "/IDLE.ANI", imageSet);
	_hit = AssetsManager::loadCopyAnimation(aniSet + "/HIT.ANI", imageSet);
	_killfall = AssetsManager::loadCopyAnimation(aniSet + "/KILLFALL.ANI", imageSet);
	_respawn = AssetsManager::loadCopyAnimation(aniSet + "/RESPAWN.ANI", imageSet);
	_slap = AssetsManager::loadCopyAnimation(aniSet + "/STRIKE1.ANI", imageSet);
	_squeeze = AssetsManager::loadCopyAnimation(aniSet + "/STRIKE3.ANI", imageSet);

	_ani = _idle;
	drawZ = DefaultZCoord::Characters + 1;
}
AquatisTentacle::~AquatisTentacle()
{
	if (removeObject)
	{
		_killfall->reset();
		GO::addObjectToActionPlane(DBG_NEW OneTimeAnimation(position, _killfall));
	}
}
void AquatisTentacle::Logic(uint32_t elapsedTime)
{
	if (_deadTime > 0)
	{
		_deadTime -= elapsedTime;

		if (_deadTime <= 0)
		{
			_ani = _respawn;
			_ani->reset();
			_ani->loopAni = false;
		}
		else
		{
			_ani->Logic(elapsedTime);
			return;
		}
	}

	if (checkForHurts())
	{
		_ani = _killfall;
		_deadTime = 5000;
		_ani->reset();
		_ani->loopAni = false;
	}
	else if (_ani->isFinishAnimation())
	{
		if (_ani == _squeeze)
		{
			_ani = _idle;
			_ani->reset();
			_squeezeRestTime = 1000;
			GO::unsqueezePlayer();
		}
		else if (_ani == _respawn || _ani == _slap)
		{
			_ani = _idle;
			_ani->reset();
		}
	}

	if (_ani == _idle)
	{
		float distance = position.x - GO::getPlayerPosition().x - 37;
		if (-55 <= distance && distance < 40)
		{
			_ani = _slap;
			_ani->reset();
			_ani->loopAni = false;
		}
		else if (-96 <= distance && distance < -55)
		{
			if (_squeezeRestTime <= 0)
			{
				if (_ani != _squeeze && !GO::isPlayerSqueezed())
				{
					_ani = _squeeze;
					_ani->reset();
					_ani->loopAni = false;
					GO::squeezePlayer({ position.x + 40, position.y - 80 });
				}
			}
			else
			{
				_squeezeRestTime -= elapsedTime;
			}
		}
	}


	_ani->Logic(elapsedTime);
}
Rectangle2D AquatisTentacle::GetRect()
{
	return _ani->GetRect();
}
bool AquatisTentacle::checkForHurts()
{
	const Rectangle2D thisRc = GetRect();

	if (_ani == _idle || _ani == _slap)
	{
		if (checkForHurt(GO::getPlayerAttackRect(), thisRc))
			return true;

		for (Projectile* p : GO::getActionPlaneProjectiles())
		{
			if (isinstance<ClawProjectile>(p))
			{
				if (p->isClawDynamite() && p->getDamage() == 0)
					continue;
				else if (checkForHurt({ p->GetRect(), p->getDamage() }, thisRc))
				{
					if (p->isClawBullet())
						p->removeObject = true;
					return true;
				}
			}
		}
	}

	return false;
}
bool AquatisTentacle::checkForHurt(pair<Rectangle2D, int> hurtData, const Rectangle2D& thisRc)
{
	if (hurtData.second > 0)
	{
		if (_lastAttackRect != hurtData.first && thisRc.intersects(hurtData.first))
		{
			_lastAttackRect = hurtData.first;
			return true;
		}
	}
	return false;
}
bool AquatisTentacle::isDamage() const
{
	return _ani == _slap;
}
