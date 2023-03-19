#include "BaseEnemy.h"
#include "Item.h"
#include "Projectile.h"
#include "../Player.h"
#include "../AssetsManager.h"
#include "../ActionPlane.h"
#include "../WindowManager.h"


#define ANIMATION_WALK			_animations.at(_walkAniName)
#define ANIMATION_STRIKE_HIGH	_animations.at(_strikeAniName)
#define ANIMATION_STRIKE_LOW	_animations.at(_strikeDuckAniName)
#define ANIMATION_HITHIGH		_animations.at(_hit1AniName)
#define ANIMATION_HITLOW		_animations.at(_hit2AniName)
#define ANIMATION_SHOOT			_animations.at(_shootAniName)
#define ANIMATION_SHOOTDUCK		_animations.at(_shootDuckAniName)


BaseEnemy::StandAniData::StandAniData(shared_ptr<Animation> ani, uint32_t duration)
	: ani(ani), duration(duration), elapsedTime(0) {}


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
	position.x += elapsedTime * _speed.x;
	position.y += elapsedTime * _speed.y;
	_speed.y += elapsedTime * GRAVITY;
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
		if (CollisionDistances::isCollision(GetRect(), _player->GetRect()))
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
// TODO: if the 'standAnisData' contains a real animation use it regulary

BaseEnemy::BaseEnemy(const WwdObject& obj, Player* player,
	int16_t health, int8_t damage, string walkAni, string hit1, string hit2,
	string fallDead, string strikeAni, string strikeDuckAni, string shootAni, string shootDuckAni,
	string projectileAniDir, vector<pair<string, uint32_t>> standAnisData, bool noTreasures)
	: BaseCharacter(obj, player), _itemsTaken(false), _damage(damage),
	_isStanding(false), _standAniIdx(0), _strikeAniName(strikeAni), _strikeDuckAniName(strikeDuckAni),
	_canStrike(!strikeAni.empty()), _canStrikeDuck(!strikeDuckAni.empty()), _walkAniName(walkAni), _shootAniName(shootAni), _canShoot(!shootAni.empty()),
	_shootDuckAniName(shootDuckAni), _canShootDuck(!shootDuckAni.empty()), _projectileAniDir(projectileAniDir),
	_hit1AniName(hit1), _hit2AniName(hit2), _fallDeadAniName(fallDead), _minX((float)obj.minX),
	_maxX((float)obj.maxX), _isStaticEnemy(obj.userValue1)
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


	if (obj.minX == 0 && obj.maxX == 0)
	{
		myMemCpy(_minX, position.x - 32);
		myMemCpy(_maxX, position.x + 32);
	}

	_speed.x = ENEMY_PATROL_SPEED;


	for (auto& a : standAnisData)
	{
		_standAni.push_back(allocNewSharedPtr<StandAniData>(_animations[a.first], a.second));
	}

	if (_isStaticEnemy)
	{
		_isStanding = true;
		if (_standAni.size() > 0)
			_ani = _standAni[0]->ani;
	}
	else
	{
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
		/*
		TODO - idle enemy animation
		
		if (is real idle animation)
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
		else
		{
			make this block
		}

		*/

		_ani = _standAni[_standAniIdx]->ani;

		if (_standAni[_standAniIdx]->elapsedTime >= _standAni[_standAniIdx]->duration)
		{
			_standAni[_standAniIdx]->elapsedTime = 0;
			_standAni[_standAniIdx]->ani->reset();

			_standAniIdx += 1;

			if (_standAniIdx == _standAni.size())
			{
				if (!_isStaticEnemy)
				{
					_isStanding = false;
					_ani = ANIMATION_WALK;
				}
				_ani->reset();
				_standAniIdx = 0;
			}
		}
		else
		{
			_standAni[_standAniIdx]->elapsedTime += elapsedTime;
		}
	}

	if (!_isStanding && !_isAttack && !_isStaticEnemy)
	{
		position.x += _speed.x * elapsedTime;
		if (position.x < _minX) { stopMovingLeft(_minX - position.x); }
		else if (position.x > _maxX) { stopMovingRight(position.x - _maxX); }
	}

	if (!_isAttack)
	{
		if (_isStaticEnemy)
		{
			_isStanding = true;
		}
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
		_standAniIdx = 0;
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

D2D1_RECT_F BaseEnemy::GetRect()
{
	_saveCurrRect = _ani->GetRect();
	return _saveCurrRect;
}

bool BaseEnemy::isStanding() const { return _isStanding; }
bool BaseEnemy::isDuck() const { return _ani == ANIMATION_SHOOTDUCK; }
bool BaseEnemy::isTakeDamage() const { return (_ani == ANIMATION_HITHIGH || _ani == ANIMATION_HITLOW) && !_ani->isFinishAnimation(); }
bool BaseEnemy::isWalkAnimation() const { return _ani == ANIMATION_WALK; }
vector<Item*> BaseEnemy::getItems()
{
	vector<Item*> items;

	if (!_itemsTaken)
	{
		WwdObject newObj;
		newObj.x = (int32_t)position.x;
		newObj.y = (int32_t)position.y;
		newObj.z = ZCoord;

		for (int8_t t : _itemsTypes)
		{
			Item* i = Item::getItem(newObj, _player, t);
			i->setSpeedY(-0.6f);
			i->setSpeedX(getRandomFloat(-0.25f, 0.25f) * (getRandomInt(0, 1) == 1 ? 1 : -1));
			items.push_back(i);
		}
		_itemsTaken = true;
	}

	return items;
}
void BaseEnemy::stopMovingLeft(float collisionSize)
{
	position.x += collisionSize;

	_speed.x = ENEMY_PATROL_SPEED;
	_forward = true;
	_isStanding = true;
}
void BaseEnemy::stopMovingRight(float collisionSize)
{
	position.x -= collisionSize;

	_speed.x = -ENEMY_PATROL_SPEED;
	_forward = false;
	_isStanding = true;
}
bool BaseEnemy::checkForHurt(pair<D2D1_RECT_F, uint8_t> hurtData)
{
	if (removeObject)
		return true;

	if (hurtData.second > 0)
	{
		if (!isTakeDamage() && _lastAttackRect != hurtData.first && CollisionDistances::isCollision(_saveCurrRect, hurtData.first))
		{
			_lastAttackRect = hurtData.first;
			_health -= hurtData.second;
			removeObject = (_health <= 0);
			return true;
		}
	}
	return false;
}
bool BaseEnemy::checkForHurts()
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
	int8_t damage, string walkAni, string hit1, string hit2, string fallDead, string strikeAni,
	string shootAni, string projectileAniDir, vector<pair<string, uint32_t>> standAnisData)
	: BaseEnemy(obj, player, obj.health, damage, walkAni, hit1, hit2, fallDead, strikeAni, "", shootAni,
		"", projectileAniDir, standAnisData, true), _gemPos({ obj.speedX, obj.speedY })
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
	throw Exception(__FUNCTION__ ": Write Me!");
	return checkForHurt(_player->GetAttackRect());
}
