#include "BaseEnemy.h"
#include "ActionPlane.h"
#include "EnemyHelp.h"
#include "PhysicsManager.h"
#include "Player.h"
#include "Objects/Item.h"
#include "Objects/EnemyProjectile.h"


BaseEnemy::BaseEnemy(const WwdObject& obj, int health, int damage, const string& walkAni,
	const string& hithigh, const string& hitlow, const string& fallDeadAni, const string& strikeAni,
	const string& strikeDuckAni, const string& shootAni, const string& shootDuckAni,
	const string& projectileAniDir, float walkingSpeed, bool noTreasures)
	: BaseCharacter(obj), _damage(damage), _isStanding(false),
	_canStrike(!strikeAni.empty()),
	_canStrikeDuck(!strikeDuckAni.empty()),
	_canShoot(!shootAni.empty()), _canShootDuck(!shootDuckAni.empty()),
	_projectileAniDir(projectileAniDir),
	_minX((float)obj.minX), _maxX((float)obj.maxX),
	_isStaticEnemy(obj.userValue1), _attackRest(0), _fallDead(true),
	_deathType(DeathType::Regular)
{
	_isMirrored = false;

	_animations = AssetsManager::loadAnimationsFromDirectory(PathManager::getAnimationSetPath(obj.imageSet), obj.imageSet);
	// add burnt and frozen animations:
	string imagesDirectoryPath = PathManager::getImageSetPath(obj.imageSet);
	_animations["burnt"] = AssetsManager::createCopyAnimationFromPidImage(imagesDirectoryPath + "/550.PID");
	_animations["frozen"] = AssetsManager::createCopyAnimationFromPidImage(imagesDirectoryPath + "/560.PID");

	_aniWalk = _animations[walkAni];
	_aniStrike = _animations[strikeAni];
	_aniStrikeDuck = _animations[strikeDuckAni];
	_aniShoot = _animations[shootAni];
	_aniShootDuck = _animations[shootDuckAni];
	_aniHitHigh = _animations[hithigh];
	_aniHitLow = _animations[hitlow];
	_aniIdle = _animations["IDLE"];
	_aniFallDead = _animations[fallDeadAni];


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
		_ani = _aniIdle;
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

		if (!walkAni.empty())
			_ani = _aniWalk;
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
		// fit enemy death animation
		switch (_deathType)
		{
		case BaseEnemy::DeathType::FireSword:
			ActionPlane::addPlaneObject(DBG_NEW OneTimeAnimation(position, "GAME_FIRESWORDEXPLOSION", "GAME_EXPLOS_FIRE"));
			ActionPlane::addPlaneObject(DBG_NEW::EnemyFallDeath(obj, _animations["burnt"]));
			break;

		case BaseEnemy::DeathType::IceSword:
			ActionPlane::addPlaneObject(DBG_NEW OneTimeAnimation(position, "GAME_ICESWORDEXPLOSION", "GAME_EXPLOS_ICE"));
			ActionPlane::addPlaneObject(DBG_NEW::EnemyFallDeath(obj, _animations["frozen"]));
			break;

		case BaseEnemy::DeathType::LightningSword:
			ActionPlane::addPlaneObject(DBG_NEW OneTimeAnimation(position, "GAME_LIGHTNINGEXPLOSION", "GAME_EXPLOS_LIGHTNING"));
			ActionPlane::addPlaneObject(DBG_NEW::EnemyFallDeath(obj, _animations["burnt"]));
			break;

		default:
			ActionPlane::addPlaneObject(DBG_NEW::EnemyFallDeath(obj, _aniFallDead));
			break;
		}

		// TODO: find enemy death sound
	}
}

void BaseEnemy::Logic(uint32_t elapsedTime)
{
	if (!PreLogic(elapsedTime)) return;

	if (_isStanding)
	{
		if (_ani != _aniIdle)
		{
			_ani = _aniIdle;
			_ani->reset();
		}
		else if (_ani->isFinishAnimation())
		{
			_isStanding = false;
			_ani = _aniWalk;
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
			_ani = _aniWalk;
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
	if (_isStanding || doesEnemySeeClaw())
		makeAttack(abs(player->position.x - position.x), abs(player->position.y - position.y));
}
void BaseEnemy::makeAttack(float deltaX, float deltaY)
{
	if (deltaX < 96 && deltaY < 16) // CC is close to enemy
	{
		if (_canStrike)
		{
			_ani = _aniStrike;
			_ani->reset();
			_isStanding = false;
			_isAttack = true;
			_isMirrored = player->position.x < position.x;
		}
		if (_canStrikeDuck && player->isDuck())
		{
			_ani = _aniStrikeDuck;
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
			_ani = _aniShootDuck;
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
			_ani = _aniShoot;
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
			_ani = _aniHitHigh;
		else
			_ani = _aniHitLow;

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
bool BaseEnemy::isDuck() const { return _ani == _aniShootDuck; }
bool BaseEnemy::isTakeDamage() const { return (_ani == _aniHitHigh || _ani == _aniHitLow) && !_ani->isFinishAnimation(); }
bool BaseEnemy::isWalkAnimation() const { return _ani == _aniWalk; }
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
		// draw damage animation
		Rectangle2D damageRc = _saveCurrRect.getCollision(clawAttackRect.first);
		OneTimeAnimation* ani = DBG_NEW OneTimeAnimation(
			{
				position.x + (damageRc.left - damageRc.right) / 2,
				position.y + (damageRc.top - damageRc.bottom) / 2
			},
			AssetsManager::createCopyAnimationFromDirectory("GAME/IMAGES/ENEMYHIT", false, 50));
		ActionPlane::addPlaneObject(ani);
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

				// fit death animation to the projectile type:
				switch (((ClawProjectile*)p)->type)
				{
				case ClawProjectile::Types::FireSword: _deathType = DeathType::FireSword; break;
				case ClawProjectile::Types::IceSword: _deathType = DeathType::IceSword; break;
				case ClawProjectile::Types::LightningSword: _deathType = DeathType::LightningSword; break;
				default: break;
				}

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

bool BaseEnemy::doesEnemySeeClaw() const
{
	return !player->isGhost() && (
		(!_isMirrored && player->position.x > position.x) ||
		(_isMirrored && player->position.x < position.x)
	);
}


BaseBoss::BaseBoss(const WwdObject& obj, const string& fallDeadAni)
	: BaseEnemy(obj, obj.health, 10, "", "HITHIGH", "HITLOW", fallDeadAni,
		"", "", "", "", "", ENEMY_PATROL_SPEED, true),
	_hitsCuonter(1), _blockClaw(false), _canJump(true), _gemPos({ obj.speedX, obj.speedY })
{
	if (fallDeadAni.empty())
		_fallDead = false;

	_ani = _aniIdle;
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
