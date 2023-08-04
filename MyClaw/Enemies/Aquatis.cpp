#include "Aquatis.h"
#include "../Assets-Managers/AssetsManager.h"
#include "../Player.h"
#include "../ActionPlane.h"


static bool shouldRespawnTentacles = false; // after the stalactite fall on Aquatis, the tentacles should respawn


Tentacle::Tentacle(const WwdObject& obj)
	: BaseDamageObject(obj, 10)
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
	myMemCpy(ZCoord, player->ZCoord + 1);
}

void Tentacle::Logic(uint32_t elapsedTime)
{
	shared_ptr<Animation> prevAni = _ani;

	if (checkForHurts())
	{
		_ani = _killfall;
		_ani->reset();
		_ani->loopAni = false;
	}
	else if ((_ani == _respawn || _ani == _slap || _ani == _squeeze) && _ani->isFinishAnimation())
	{
		_ani = _idle;
		_ani->reset();
	}

	if (shouldRespawnTentacles)
	{
		_ani = _respawn;
		_ani->reset();
		_ani->loopAni = false;
	}

	if (_ani == _idle)
	{
		float distance = position.x - player->position.x-37;
		if (-55 <= distance && distance < 64)
		{
			_ani = _slap;
			_ani->reset();
			_ani->loopAni = false;
		}
		else if (-96 <= distance && distance < -55)
		{
			_ani = _squeeze;
			_ani->reset();
			_ani->loopAni = false;
		}
	}


	_ani->Logic(elapsedTime);
}

Rectangle2D Tentacle::GetRect()
{
	return _ani->GetRect();
}


// TODO: same to BaseEnemy, maybe we can combine them
bool Tentacle::checkForHurts()
{
	auto checkForHurt = [&](pair<Rectangle2D, int> hurtData)
	{
		if (hurtData.second > 0)
		{
			if (_lastAttackRect != hurtData.first && GetRect().intersects(hurtData.first))
			{
				_lastAttackRect = hurtData.first;
				return true;
			}
		}
		return false;
	};

	if (_ani == _idle || _ani == _slap)
	{
		if (checkForHurt(player->GetAttackRect()))
			return true;

		for (Projectile* p : ActionPlane::getProjectiles())
		{
			if (isClawProjectile(p))
			{
				if (p->isClawDynamite() && p->getDamage() == 0)
					continue;
				else if (checkForHurt({ p->GetRect(), p->getDamage() }))
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

bool Tentacle::isDamage() const
{
	return _ani == _slap || _ani == _squeeze;
}
