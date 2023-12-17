#include "BaseEnemy.h"
#include "ActionPlane.h"
#include "EnemyHelp.h"
#include "PhysicsManager.h"
#include "Player.h"
#include "Assets-Managers/AssetsManager.h"
#include "Objects/Item.h"
#include "Objects/EnemyProjectile.h"


#define ANIMATION_IDLE			_animations.at(_idleAniName)
#define ANIMATION_WALK			_animations.at(_walkAniName)
#define ANIMATION_STRIKE_HIGH	_animations.at(_strikeAniName)
#define ANIMATION_STRIKE_LOW	_animations.at(_strikeDuckAniName)
#define ANIMATION_HITHIGH		_animations.at(_hitHighAniName)
#define ANIMATION_HITLOW		_animations.at(_hitLowAniName)
#define ANIMATION_SHOOT			_animations.at(_shootAniName)
#define ANIMATION_SHOOTDUCK		_animations.at(_shootDuckAniName)


// TODO: fix the shoot to CC height

BaseEnemy::BaseEnemy(const WwdObject& obj, int health, int damage, const string& walkAni,
	const string& hithigh, const string& hitlow, const string& fallDead, const string& strikeAni,
	const string& strikeDuckAni, const string& shootAni, const string& shootDuckAni,
	const string& projectileAniDir, float walkingSpeed, bool noTreasures)
	: BaseCharacter(obj), _damage(damage), _isStanding(false), _strikeAniName(strikeAni),
	_strikeDuckAniName(strikeDuckAni), _canStrike(!strikeAni.empty()),
	_canStrikeDuck(!strikeDuckAni.empty()), _walkAniName(walkAni), _shootAniName(shootAni),
	_canShoot(!shootAni.empty()), _shootDuckAniName(shootDuckAni), _canShootDuck(!shootDuckAni.empty()),
	_projectileAniDir(projectileAniDir), _hitHighAniName(hithigh), _hitLowAniName(hitlow),
	_fallDeadAniName(fallDead), _minX((float)obj.minX), _maxX((float)obj.maxX),
	_isStaticEnemy(obj.userValue1), _idleAniName("IDLE"), _attackRest(0), _fallDead(true)
{
	_isMirrored = false;
	_animations = AssetsManager::loadAnimationsFromDirectory(PathManager::getAnimationSetPath(obj.imageSet), obj.imageSet);
	_health = health;

	if (obj.powerup > 0) _itemsTypes.push_back(obj.powerup);
	if (obj.userRect1.left > 0) _itemsTypes.push_back(obj.userRect1.left);
	if (obj.userRect1.right > 0) _itemsTypes.push_back(obj.userRect1.right);
	if (obj.userRect1.bottom > 0) _itemsTypes.push_back(obj.userRect1.bottom);
	if (obj.userRect1.top > 0) _itemsTypes.push_back(obj.userRect1.top);
	if (obj.userRect2.left > 0) _itemsTypes.push_back(obj.userRect2.left);
	if (obj.userRect2.right > 0) _itemsTypes.push_back(obj.userRect2.right);
	if (obj.userRect2.bottom > 0) _itemsTypes.push_back(obj.userRect2.bottom);
	if (obj.userRect2.top > 0) _itemsTypes.push_back(obj.userRect2.top);
	if (!noTreasures && _itemsTypes.size() == 0) _itemsTypes.push_back(Item::Type::Treasure_Coins);


	speed.x = walkingSpeed;

	if (_isStaticEnemy)
	{
		_isStanding = true;
		if (!_idleAniName.empty())
			_ani = ANIMATION_IDLE;
	}
	else
	{
		if (obj.logic != "Seagull" && obj.logic != "Fish")
		{
			// find enemy range
			pair<float, float> range = physics->getEnemyRange(position, _minX, _maxX);
			myMemCpy(_minX, range.first);
			myMemCpy(_maxX, range.second);
		}

		if (!_walkAniName.empty())
			_ani = ANIMATION_WALK;
		//else
		//	_ani = ANIMATION_IDLE; // TODO: breakpoint here
	}
}
BaseEnemy::~BaseEnemy()
{
	if (removeObject && _fallDead)
	{
		WwdObject obj;
		obj.x = (int32_t)position.x;
		obj.y = (int32_t)position.y;

		// add items
		for (int8_t t : _itemsTypes)
		{
			Item* i = Item::getItem(obj, t);
			i->speed.y = -0.6f;
			i->speed.x = getRandomFloat(-0.25f, 0.25f);
			ActionPlane::addPlaneObject(i);
		}

		// add dead enemy
		obj.speedX = position.x < player->position.x ? -250 : 250;
		obj.speedY = -500;
		ActionPlane::addPlaneObject(DBG_NEW ::EnemyFallDeath(obj, _animations[_fallDeadAniName]));
		// TODO: fit enemy death animation to the enemy (regular, ice, etc.)
	}
}

void BaseEnemy::Logic(uint32_t elapsedTime)
{
	if (!PreLogic(elapsedTime)) return;

	if (_isStanding)
	{
		if (_ani != ANIMATION_IDLE)
		{
			_ani = ANIMATION_IDLE;
			_ani->reset();
		}
		else if (_ani->isFinishAnimation())
		{
			_isStanding = false;
			_ani = ANIMATION_WALK;
			_ani->reset();
		}
	}

	if (!_isStanding && !_isAttack && !_isStaticEnemy)
	{
		position.x += speed.x * elapsedTime;
		if (position.x < _minX) { stopMovingLeft(_minX - position.x); }
		else if (position.x > _maxX) { stopMovingRight(position.x - _maxX); }
	}

	speed.y += GRAVITY * elapsedTime;
	position.y += speed.y * elapsedTime;


	if (_isAttack)
	{
		if (_ani->isFinishAnimation())
		{
			_ani = ANIMATION_WALK;
			_ani->reset();
			_isAttack = false;
			_isMirrored = speed.x < 0;
		}
	}
	else
	{
		if (_attackRest > 0)
			_attackRest -= elapsedTime;

		if (_isStaticEnemy)
		{
			_isStanding = true;
		}
		if (_attackRest <= 0)
			makeAttack();
	}

	PostLogic(elapsedTime);
}
void BaseEnemy::makeAttack()
{
	if (_isStanding || enemySeeClaw())
		makeAttack(abs(player->position.x - position.x), abs(player->position.y - position.y));
}
void BaseEnemy::makeAttack(float deltaX, float deltaY)
{
	if (deltaX < 96 && deltaY < 16) // CC is close to enemy
	{
		if (_canStrike)
		{
			_ani = ANIMATION_STRIKE_HIGH;
			_ani->reset();
			_isStanding = false;
			_isAttack = true;
			_isMirrored = player->position.x < position.x;
		}
		if (_canStrikeDuck && player->isDuck())
		{
			_ani = ANIMATION_STRIKE_LOW;
			_ani->reset();
			_isStanding = false;
			_isAttack = true;
			_isMirrored = player->position.x < position.x;
		}
	}
	else if (deltaX < 256) // CC is far from enemy
	{
		if (_canShootDuck && deltaY < 128 && player->isDuck())
		{
			_ani = ANIMATION_SHOOTDUCK;
			_ani->reset();
			_isStanding = false;
			_isAttack = true;
			_isMirrored = player->position.x < position.x;

			WwdObject obj;
			obj.x = (int32_t)(position.x + (!_isMirrored ? _saveCurrRect.right - _saveCurrRect.left : _saveCurrRect.left - _saveCurrRect.right));
			obj.y = (int32_t)position.y + 10;
			obj.speedX = !_isMirrored ? DEFAULT_PROJECTILE_SPEED : -DEFAULT_PROJECTILE_SPEED;
			obj.damage = 10;
			ActionPlane::addPlaneObject(DBG_NEW EnemyProjectile(obj, _projectileAniDir));
		}
		else if (_canShoot && deltaY < 16)
		{
			_ani = ANIMATION_SHOOT;
			_ani->reset();
			_isStanding = false;
			_isAttack = true;
			_isMirrored = player->position.x < position.x;

			WwdObject obj;
			obj.x = (int32_t)(position.x + (!_isMirrored ? _saveCurrRect.right - _saveCurrRect.left : _saveCurrRect.left - _saveCurrRect.right));
			obj.y = (int32_t)position.y - 20;
			obj.speedX = !_isMirrored ? DEFAULT_PROJECTILE_SPEED : -DEFAULT_PROJECTILE_SPEED;
			obj.damage = 10;
			ActionPlane::addPlaneObject(DBG_NEW EnemyProjectile(obj, _projectileAniDir));
		}
	}
}

bool BaseEnemy::PreLogic(uint32_t elapsedTime)
{
	if (isTakeDamage() && !_ani->isFinishAnimation())
	{
		_ani->Logic(elapsedTime);
		return false;
	}

	if (checkForHurts())
	{
		// match hit-high / hit-low to attack height
		//if (_saveCurrRect.top + (_saveCurrRect.top - _saveCurrRect.bottom) / 2 < _lastAttackRect.top)
		if (3 * _saveCurrRect.top - _saveCurrRect.bottom < 2 * _lastAttackRect.top) // the magic of math :)
			_ani = ANIMATION_HITHIGH;
		else
			_ani = ANIMATION_HITLOW;
		
		_ani->reset();
		_ani->loopAni = false;
		_ani->mirrored = !_isMirrored;
		_ani->position = position;
		_isAttack = false;
		_isStanding = true;
		_lastAttackRect = {};
		return false;
	}
	
	return true;
}
void BaseEnemy::PostLogic(uint32_t elapsedTime)
{
	_ani->mirrored = !_isMirrored;
	_ani->position = position;
	_ani->Logic(elapsedTime);
}

Rectangle2D BaseEnemy::GetRect()
{
	_saveCurrRect = _ani->GetRect();
	return _saveCurrRect;
}

bool BaseEnemy::isStanding() const { return _isStanding; }
bool BaseEnemy::isDuck() const { return _ani == ANIMATION_SHOOTDUCK; }
bool BaseEnemy::isTakeDamage() const { return (_ani == ANIMATION_HITHIGH || _ani == ANIMATION_HITLOW) && !_ani->isFinishAnimation(); }
bool BaseEnemy::isWalkAnimation() const { return _ani == ANIMATION_WALK; }
void BaseEnemy::stopMovingLeft(float collisionSize)
{
	position.x += collisionSize;
	speed.x = -speed.x;
	_isMirrored = false;
	_isStanding = true;
}
void BaseEnemy::stopMovingRight(float collisionSize)
{
	position.x -= collisionSize;
	speed.x = -speed.x;
	_isMirrored = true;
	_isStanding = true;
}
bool BaseEnemy::checkForHurt(const pair<Rectangle2D, int>& hurtData)
{
	if (removeObject)
		return true;

	if (hurtData.second > 0)
	{
		if (!isTakeDamage() && _lastAttackRect != hurtData.first && _saveCurrRect.intersects(hurtData.first))
		{
			_lastAttackRect = hurtData.first;
			_health -= hurtData.second;
			removeObject = (_health <= 0);
			return true;
		}
	}
	return false;
}
bool BaseEnemy::checkClawHit()
{
	pair<Rectangle2D, int> clawAttackRect = player->GetAttackRect();

	if (checkForHurt(clawAttackRect))
	{
#ifndef LOW_DETAILS
		// draw damage animation
		Rectangle2D damageRc = _saveCurrRect.getCollision(clawAttackRect.first);
		OneTimeAnimation* ani = DBG_NEW OneTimeAnimation(
			{
				position.x + (damageRc.left - damageRc.right) / 2,
				position.y + (damageRc.top - damageRc.bottom) / 2
			},
			AssetsManager::createCopyAnimationFromDirectory("GAME/IMAGES/ENEMYHIT", false, 50));
		myMemCpy(ani->logicZ, logicZ + 1);
		ActionPlane::addPlaneObject(ani);
#endif
		return true;
	}
	return false;
}
bool BaseEnemy::checkForHurts()
{
	if (checkClawHit())
		return true;

	for (Projectile* p : ActionPlane::getProjectiles())
	{
		if (isbaseinstance<ClawProjectile>(p))
		{
			if (p->isClawDynamite() && p->getDamage() == 0) continue;
			if (checkForHurt({ p->GetRect(), p->getDamage() }))
			{
				if (p->isClawBullet())
					p->removeObject = true;
				return true;
			}
		}
	}

	for (PowderKeg* p : ActionPlane::getPowderKegs())
	{
		if (checkForHurt({ p->GetRect(), p->getDamage() }))
		{
			return true;
		}
	}

	return false;
}

bool BaseEnemy::enemySeeClaw() const
{
	return !player->isGhost() && (
		(!_isMirrored && player->position.x > position.x) ||
		(_isMirrored && player->position.x < position.x)
		);
}


// TODO: maybe this c'tor doesn't need get parameters...
BaseBoss::BaseBoss(const WwdObject& obj, int damage, 
	const string& walkAni, const string& hithigh, const string& hitlow, const string& fallDead,
	const string& strikeAni, const string& shootAni, const string& projectileAniDir)
	: BaseEnemy(obj, obj.health, damage, walkAni, hithigh, hitlow, fallDead,
		strikeAni, "", shootAni, "", projectileAniDir, ENEMY_PATROL_SPEED, true),
	_hitsCuonter(1), _blockClaw(false), _canJump(true), _gemPos({ obj.speedX, obj.speedY })
{
}
BaseBoss::~BaseBoss()
{
	if (removeObject)
	{
		WwdObject obj;
		obj.x = (int32_t)position.x;
		obj.y = (int32_t)position.y;
		obj.minX = _gemPos.x;
		obj.minY = _gemPos.y;
		obj.imageSet = "LEVEL_GEM";
		ActionPlane::addPlaneObject(DBG_NEW BossGem(obj));
	}
}
