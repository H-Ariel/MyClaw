#include "Aquatis.h"
#include "AquatisTentacle.h"
#include "../../../GlobalObjects.h"
#include "../../../Objects/OneTimeAnimation.h"
#include "../../../Objects/Projectiles/ClawProjectile.h"
#include "../../../Objects/Player/Player.h"


AquatisTentacle::AquatisTentacle(const WwdObject& obj)
	: BaseDamageObject(obj, 5), _squeezeTimer(1000),
	_deadTimer(bind(&AquatisTentacle::respawn, this))
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

	addTimer(&_deadTimer);
	addTimer(&_squeezeTimer);
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
	if (_deadTimer.isFinished()) {

		if (checkForHurts())
		{
			_ani = _killfall;
			_deadTimer.reset(5000);
			addTimer(&_deadTimer);
			_ani->reset();
			_ani->loopAni = false;
		}
		else if (_ani->isFinishAnimation())
		{
			if (_ani == _squeeze)
			{
				_ani = _idle;
				_ani->reset();
				_squeezeTimer.reset();
				addTimer(&_squeezeTimer);
				GO::player->unsqueeze();
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
				if (_squeezeTimer.isFinished())
				{
					if (_ani != _squeeze && !GO::isPlayerSqueezed())
					{
						_ani = _squeeze;
						_ani->reset();
						_ani->loopAni = false;
						GO::player->squeeze({ position.x + 40, position.y - 80 });
					}
				}
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

void AquatisTentacle::respawn()
{
	_ani = _respawn;
	_ani->reset();
	_ani->loopAni = false;
}
