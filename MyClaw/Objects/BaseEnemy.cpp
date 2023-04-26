#include "BaseEnemy.h"
#include "Item.h"
#include "Projectile.h"
#include "../Player.h"
#include "../AssetsManager.h"
#include "../ActionPlane.h"
#include "../WindowManager.h"


#define ANIMATION_IDLE			_animations.at(_idleAniName)
#define ANIMATION_WALK			_animations.at(_walkAniName)
#define ANIMATION_STRIKE_HIGH	_animations.at(_strikeAniName)
#define ANIMATION_STRIKE_LOW	_animations.at(_strikeDuckAniName)
#define ANIMATION_HITHIGH		_animations.at(_hit1AniName)
#define ANIMATION_HITLOW		_animations.at(_hit2AniName)
#define ANIMATION_SHOOT			_animations.at(_shootAniName)
#define ANIMATION_SHOOTDUCK		_animations.at(_shootDuckAniName)


DeadEnemy::DeadEnemy(const WwdObject& obj, shared_ptr<Animation> deadAni)
	: BaseDynamicPlaneObject(obj)
{
	_ani = deadAni;
	_speed.x = obj.speedX / 1000.f;
	_speed.y = obj.speedY / 1000.f;
	_isMirrored = obj.speedX < 0;
}
void DeadEnemy::Logic(uint32_t elapsedTime)
{
	position.x += _speed.x * elapsedTime;
	_speed.y += GRAVITY * elapsedTime;
	position.y += _speed.y * elapsedTime;
	_ani->position = position;
	_ani->Logic(elapsedTime);
	removeObject = !WindowManager::isInScreen(_ani->GetRect());
}

BossGem::BossGem(const WwdObject& obj, Player* player)
	: Item(obj, player, Item::Type::NineLivesGem), _destination({ (float)obj.minX, (float)obj.minY })
{
	float alpha = atan((position.y - _destination.y) / (position.x - _destination.x));

	_speed.x = cos(alpha) * GEM_SPEED;
	_speed.y = sin(alpha) * GEM_SPEED;

	if (_destination.x == position.x)
	{
		_speed.x = 0;
		_speed.y = GEM_SPEED;
	}
	else if (_destination.x < position.x)
	{
		_speed.x = -_speed.x;
		_speed.y = -_speed.y;
	}
}
void BossGem::Logic(uint32_t elapsedTime)
{
	if (_speed.x == 0 && _speed.y == 0)
	{
		if (GetRect().intersects(_player->GetRect()))
		{
			_player->collectItem(this);
		}
	}

	if (abs(position.x - _destination.x) <= 5 && abs(position.y - _destination.y) <= 5)
	{
		position = _destination;
		_speed = {};
	}
	else
	{
		position.y += _speed.y * elapsedTime;
		position.x += _speed.x * elapsedTime;
	}
}

// TODO: fit 'hithigh' and 'hitlow' to CC attack
// TODO: fix the shoot to CC height

BaseEnemy::BaseEnemy(const WwdObject& obj, Player* player,
	int health, int damage, string walkAni, string hit1, string hit2,
	string fallDead, string strikeAni, string strikeDuckAni, string shootAni, string shootDuckAni,
	string projectileAniDir, float walkingSpeed, bool noTreasures)
	: BaseCharacter(obj, player), _damage(damage), _forward(true),
	_isStanding(false), _strikeAniName(strikeAni), _strikeDuckAniName(strikeDuckAni),
	_canStrike(!strikeAni.empty()), _canStrikeDuck(!strikeDuckAni.empty()), _walkAniName(walkAni), _shootAniName(shootAni), _canShoot(!shootAni.empty()),
	_shootDuckAniName(shootDuckAni), _canShootDuck(!shootDuckAni.empty()), _projectileAniDir(projectileAniDir),
	_hit1AniName(hit1), _hit2AniName(hit2), _fallDeadAniName(fallDead), _minX((float)obj.minX),
	_maxX((float)obj.maxX), _isStaticEnemy(obj.userValue1), _idleAniName("IDLE"), _attackRest(0)
{
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


	_speed.x = walkingSpeed;

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
			auto range = ActionPlane::getPhysicsManager().getEnemyRange(position, _minX, _maxX);
			myMemCpy(_minX, range.first);
			myMemCpy(_maxX, range.second);
		}

		if (!_walkAniName.empty())
			_ani = ANIMATION_WALK;
	}
}
BaseEnemy::~BaseEnemy()
{
	if (removeObject)
	{
		WwdObject obj;
		obj.x = (int32_t)position.x;
		obj.y = (int32_t)position.y;
		obj.z = ZCoord;

		// add items
		for (int8_t t : _itemsTypes)
		{
			Item* i = Item::getItem(obj, _player, t);
			i->setSpeedY(-0.6f);
			i->setSpeedX(getRandomFloat(-0.25f, 0.25f));
			ActionPlane::addPlaneObject(i);
		}

		// add dead enemy
		obj.speedX = position.x < _player->position.x ? -250 : 250;
		obj.speedY = -500;
		ActionPlane::addPlaneObject(DBG_NEW DeadEnemy(obj, _animations[_fallDeadAniName]));
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
		position.x += _speed.x * elapsedTime;
		if (position.x < _minX) { stopMovingLeft(_minX - position.x); }
		else if (position.x > _maxX) { stopMovingRight(position.x - _maxX); }
	}

	_speed.y += GRAVITY * elapsedTime;
	position.y += _speed.y * elapsedTime;
	

	if (!_isAttack) // TODO: replace blocks and do not use `!`
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
	else
	{
		if (_ani->isFinishAnimation())
		{
			_ani = ANIMATION_WALK;
			_ani->reset();
			_isAttack = false;
			_forward = _speed.x > 0;
		}
	}

	PostLogic(elapsedTime);
}
void BaseEnemy::makeAttack()
{
	const bool isInRange = (_forward && _player->position.x > position.x) || (!_forward && _player->position.x < position.x);

	if (_isStanding || isInRange)
	{
		const float deltaX = abs(_player->position.x - position.x), deltaY = abs(_player->position.y - position.y);

		if (deltaX < 96 && deltaY < 16) // CC is close to enemy
		{
			if (_canStrike)
			{
				_ani = ANIMATION_STRIKE_HIGH;
				_ani->reset();
				_isStanding = false;
				_isAttack = true;
				_forward = _player->position.x > position.x;
			}
			if (_canStrikeDuck && _player->isDuck())
			{
				_ani = ANIMATION_STRIKE_LOW;
				_ani->reset();
				_isStanding = false;
				_isAttack = true;
				_forward = _player->position.x > position.x;
			}
		}
		else if (deltaX < 256) // CC is far from enemy
		{
			if (_canShoot && deltaY < 16)
			{
				_ani = ANIMATION_SHOOT;
				_ani->reset();
				_isStanding = false;
				_isAttack = true;
				_forward = _player->position.x > position.x;

				WwdObject obj;
				obj.x = (int32_t)(position.x + (_forward ? _saveCurrRect.right - _saveCurrRect.left : _saveCurrRect.left - _saveCurrRect.right));
				obj.y = (int32_t)position.y - 20;
				obj.z = ZCoord;
				obj.speedX = _forward ? DEFAULT_PROJECTILE_SPEED : -DEFAULT_PROJECTILE_SPEED;
				obj.damage = 10;
				ActionPlane::addPlaneObject(DBG_NEW EnemyProjectile(obj, _projectileAniDir));
			}
			else if (_canShootDuck && deltaY < 128 && _player->isDuck())
			{
				_ani = ANIMATION_SHOOTDUCK;
				_ani->reset();
				_isStanding = false;
				_isAttack = true;
				_forward = _player->position.x > position.x;

				WwdObject obj;
				obj.x = (int32_t)(position.x + (_forward ? _saveCurrRect.right - _saveCurrRect.left : _saveCurrRect.left - _saveCurrRect.right));
				obj.y = (int32_t)position.y + 10;
				obj.z = ZCoord;
				obj.speedX = _forward ? DEFAULT_PROJECTILE_SPEED : -DEFAULT_PROJECTILE_SPEED;
				obj.damage = 10;
				ActionPlane::addPlaneObject(DBG_NEW EnemyProjectile(obj, _projectileAniDir));
			}
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
		_ani = getRandomInt(0, 1) == 1 ? ANIMATION_HITLOW : ANIMATION_HITHIGH;
		_ani->reset();
		_ani->loopAni = false;
		_ani->mirrored = _forward;
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
	_ani->mirrored = _forward;
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
	_speed.x = -_speed.x;
	_forward = true;
	_isStanding = true;
}
void BaseEnemy::stopMovingRight(float collisionSize)
{
	position.x -= collisionSize;
	_speed.x = -_speed.x;
	_forward = false;
	_isStanding = true;
}
bool BaseEnemy::checkForHurt(pair<Rectangle2D, int> hurtData)
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
bool BaseEnemy::checkForHurts() // TODO: combine all `checkForHurts` methods from all enemies
{
	if (checkForHurt(_player->GetAttackRect()))
		return true;

	for (Projectile* p : ActionPlane::getProjectiles())
	{
		if (isClawProjectile(p))
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


// TODO: maybe this c'tor don't need get parameters...
BaseBoss::BaseBoss(const WwdObject& obj, Player* player,
	int damage, string walkAni, string hit1, string hit2, string fallDead,
	string strikeAni, string shootAni, string projectileAniDir)
	: BaseEnemy(obj, player, obj.health, damage, walkAni, hit1, hit2, fallDead,
		strikeAni, "", shootAni, "", projectileAniDir, 0, true),
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
		obj.z = ZCoord;
		obj.imageSet = "LEVEL_GEM";
		ActionPlane::addPlaneObject(DBG_NEW BossGem(obj, _player));
	}
}
bool BaseBoss::checkForHurts()
{
	//throw Exception(__FUNCTION__ ": Write Me!");
	throw Exception("not implemented");
//	return checkForHurt(_player->GetAttackRect());

	// TODO: maybe- `if BaseEnemy::checkForHurts(): block`
	// or- implement this function (copy from other bosses)
}
