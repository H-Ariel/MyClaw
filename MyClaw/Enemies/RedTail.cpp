#include "RedTail.h"
#include "../Player.h"
#include "../ActionPlane.h"


#define ANIMATION_BLOCK _animations["BLOCK"]


static RedTailWind* _wind = nullptr;


// TODO: write real logic for this boss (combine Red-Tail and wind)
// TODO: ad original death animation
RedTail::RedTail(const WwdObject& obj)
	: BaseBoss(obj, "KILLFALL"), _windTimeCounter(0)
{
	_health = 100;
	_aniWalk = _animations["FASTADVANCE"];
	_aniStrike = _animations["STRIKE8"];
	(bool&)_canStrike = true;
	_aniShoot = _animations["STRIKE4"];
	(bool&)_canShoot = true;
	(string&)_projectileAniDir = "LEVEL_REDTAILBULLET";

	_ani = _aniWalk;
}
void RedTail::Logic(uint32_t elapsedTime)
{
	BaseEnemy::Logic(elapsedTime);

	_wind->Logic(elapsedTime);

	_windTimeCounter -= elapsedTime;
	if (_windTimeCounter <= 0)
	{
		_wind->activate();
		_windTimeCounter = 6000;
	}
}
pair<Rectangle2D, int> RedTail::GetAttackRect()
{
	if (_ani == _aniStrike)
	{
		Rectangle2D rc;
		rc.top = position.y - 20;
		rc.bottom = position.y + 20;

		if (_isMirrored)
		{
			rc.left = position.x - 68;
			rc.right = position.x - 28;
		}
		else {
			rc.left = position.x + 28;
			rc.right = position.x + 68;
		}

		return { rc, 10 };
	}

	return {};
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


RedTailWind::RedTailWind(const WwdObject& obj)
	: BasePlaneObject(obj), _windTimeCounter(0), _windSoundId(AssetsManager::INVALID_AUDIOPLAYER_ID)
{
	_wind = this;
	logicZ = DefaultZCoord::Characters - 1; // before the player so he can stop moving left when he arrives to the spikes
}
RedTailWind::~RedTailWind() { _wind = nullptr; }
void RedTailWind::Logic(uint32_t elapsedTime)
{
	if (_windTimeCounter > 0)
	{
		_windTimeCounter -= elapsedTime;
		if (player->position.x > 37122) // make sure player do not pass the spikes
			player->position.x -= 0.1f * elapsedTime;
		if (_windSoundId == AssetsManager::INVALID_AUDIOPLAYER_ID)
		{
			_windSoundId = AssetsManager::playWavFile("LEVEL13/SOUNDS/REDTAIL/WINDWHISTLING.WAV", 50);
		}
	}
	else if (_windSoundId != AssetsManager::INVALID_AUDIOPLAYER_ID)
	{
		AssetsManager::stopWavFile(_windSoundId);
	}
}
void RedTailWind::activate() { _windTimeCounter = 3000; }
