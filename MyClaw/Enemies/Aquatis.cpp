#include "Aquatis.h"
#include "../Assets-Managers/AssetsManager.h"
#include "../Player.h"
#include "../ActionPlane.h"
#include "../Objects/Item.h"


#define ANIMATION_HITHIGH	_animations["HITHIGH"]
#define ANIMATION_HITLOW	_animations["HITLOW"]
#define ANIMATION_KILLFALL	_animations["KILLFALL"]
#define ANIMATION_STRIKE1	_animations["STRIKE1"] // punch up
#define ANIMATION_STRIKE2	_animations["STRIKE2"] // punch down


static int activateAquatisStalactite = 0;
static vector<AquatisStalactite*> AquatisStalactitesList; // list of stalactites that can hurt Aquatis
static vector<AquatisTentacle*> AquatisTentaclesList; // list of tentacles that can hurt Aquatis


Aquatis::Aquatis(const WwdObject& obj)
	: BaseBoss(obj, 0, "", "HITHIGH", "HITLOW", "KILLFALL", "STRIKE1", "", "")
{
	_ani = _animations["IDLE1"];
	_isMirrored = true;
}
Aquatis::~Aquatis()
{
	if (removeObject)
	{
		_fallDead = false;
		ActionPlane::addPlaneObject(DBG_NEW OneTimeAnimation(position, ANIMATION_KILLFALL));

		for (AquatisTentacle* i : AquatisTentaclesList)
			i->removeObject = true;
	}
}
void Aquatis::Logic(uint32_t elapsedTime)
{
	if (AquatisStalactitesList.size() == 0)
	{
		removeObject = true;
		return;
	}

	if (isTakeDamage() && !_ani->isFinishAnimation())
	{
		PostLogic(elapsedTime);
		return;
	}

	_ani = _animations["IDLE1"];

	Rectangle2D rc = GetRect();
	for (AquatisStalactite* i : AquatisStalactitesList)
	{
		if (rc.intersects(i->GetRect()))
		{
			i->stopFalling(0);
			_ani = getRandomInt(0, 1) == 1 ? ANIMATION_HITLOW : ANIMATION_HITHIGH;
			_ani->reset();
			_ani->loopAni = false;
		}
	}


	// TODO: punch CC after hit-animation is finished (and use tentacles)


	PostLogic(elapsedTime);
}
pair<Rectangle2D, int> Aquatis::GetAttackRect()
{
	return pair<Rectangle2D, int>();
}
bool Aquatis::checkForHurts()
{
	return false;
}


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
	myMemCpy(ZCoord, player->ZCoord + 1);

	AquatisTentaclesList.push_back(this);
}
AquatisTentacle::~AquatisTentacle()
{
	if (removeObject)
	{
		ActionPlane::addPlaneObject(DBG_NEW OneTimeAnimation(position, _killfall));
		AquatisTentaclesList.erase(find(AquatisTentaclesList.begin(), AquatisTentaclesList.end(), this));
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
			player->unsqueeze();
		}
		else if (_ani == _respawn || _ani == _slap)
		{
			_ani = _idle;
			_ani->reset();
		}
	}

	if (_ani == _idle)
	{
		float distance = position.x - player->position.x - 37;
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
				if (_ani != _squeeze && !player->isSqueezed())
				{
					_ani = _squeeze;
					_ani->reset();
					_ani->loopAni = false;
					player->squeeze({ position.x + 40, position.y - 80 });
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
	// TODO: same to BaseEnemy, maybe we can combine them

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
bool AquatisTentacle::isDamage() const
{
	return _ani == _slap /*|| _ani == _squeeze*/;
}


AquatisCrack::AquatisCrack(const WwdObject& obj)
	: BaseStaticPlaneObject(obj), _lastDynamite(nullptr)
{
	_ani = AssetsManager::createAnimationFromDirectory(PathManager::getImageSetPath(obj.imageSet));
	setObjectRectangle();
}
void AquatisCrack::Logic(uint32_t elapsedTime)
{
	for (Projectile* p : ActionPlane::getProjectiles())
	{
		if (p != _lastDynamite && p->isClawDynamite() &&
			p->getDamage() && _objRc.intersects(p->GetRect()))
		{
			_lastDynamite = p;
			activateAquatisStalactite += 1;
		}
	}
}


AquatisDynamite::AquatisDynamite(const WwdObject& obj)
	: BaseStaticPlaneObject(obj), _timeCounter(0) {}
void AquatisDynamite::Logic(uint32_t elapsedTime)
{
	_timeCounter -= elapsedTime;
	if (_timeCounter <= 0)
	{
		_timeCounter = 10000; // every 10 seconds respawn the dynamite supply

		WwdObject obj;
		obj.x = (int32_t)position.x;
		obj.y = (int32_t)position.y;
		ActionPlane::addPlaneObject(Item::getItem(obj, Item::Ammo_Dynamite));

		// TODO: if the previues dynamite is not taken, remove it
	}
}


// this object same to regular stalactite but with different logic
AquatisStalactite::AquatisStalactite(const WwdObject& obj)
	: Projectile(obj, ""), _idx(obj.smarts)
{
	_ani = AssetsManager::createCopyAnimationFromDirectory(PathManager::getImageSetPath(obj.imageSet), false, 50);
	_ani->updateFrames = false;
	speed = {};
	AquatisStalactitesList.push_back(this);
}
AquatisStalactite::~AquatisStalactite()
{
	AquatisStalactitesList.erase(find(AquatisStalactitesList.begin(), AquatisStalactitesList.end(), this));
}
void AquatisStalactite::Logic(uint32_t elapsedTime)
{
	if (speed.y == 0)
	{
		if (activateAquatisStalactite == _idx)
		{
			speed.y = 0.25f;
		}
	}
	else
	{
		speed.y += GRAVITY * elapsedTime;
		position.y += speed.y * elapsedTime;
	}

	_ani->position = position;
	_ani->Logic(elapsedTime);
	removeObject = _ani->isFinishAnimation();
}
void AquatisStalactite::stopFalling(float collisionSize) { _ani->updateFrames = true; }
int AquatisStalactite::getDamage() const { return 10; }
