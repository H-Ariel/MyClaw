#pragma once

#include "../BaseCharacter.h"
#include "Item.h"


#define ENEMY_PATROL_SPEED	0.1f // TODO: make it a parameter in ctor (each enemy has its own speed)
#define GEM_SPEED			0.2f


class BaseEnemy : public BaseCharacter
{
public:
	BaseEnemy(const WwdObject& obj, Player* player, int health,
		int damage, string walkAni, string hit1, string hit2,
		string fallDead, string strikeAni, string strikeDuckAni,
		string shootAni, string shootDuckAni, string projectileAniDir,
		float walkingSpeed, bool noTreasures = false);
	~BaseEnemy();

	virtual void Logic(uint32_t elapsedTime) override;
	virtual Rectangle2D GetRect() override;

	bool isStanding() const override;
	bool isDuck() const override;
	bool isTakeDamage() const override;

	void stopMovingLeft(float collisionSize) override;
	void stopMovingRight(float collisionSize) override;


protected:
	virtual void makeAttack();

	virtual bool PreLogic(uint32_t elapsedTime); // called before `Logic`. returns `true` if we need do `Logic`
	virtual void PostLogic(uint32_t elapsedTime); // called after `Logic`


	bool isWalkAnimation() const;
	bool checkForHurt(pair<Rectangle2D, int> hurtData); // returns `true` if the enemy hurt. `hurtData`={rect,damage}
	virtual bool checkForHurts();


	const string _walkAniName, _hit1AniName, _hit2AniName,
		_fallDeadAniName, _strikeAniName, _strikeDuckAniName,
		_shootAniName, _shootDuckAniName, _projectileAniDir, _idleAniName;
	vector<int8_t> _itemsTypes;
	const float _minX, _maxX;
	int32_t _attackRest; // rest time between attack. NOTE: not all enemies used that
	int _damage; // the amount of health that enemy took when he hit Claw
	bool _isStanding;
	bool _forward; // TODO use `_isMirrored`
	const bool _canStrike, _canStrikeDuck, _canShoot, _canShootDuck;
	const bool _isStaticEnemy; // it always idle
};


class BaseBoss : public BaseEnemy
{
public:
	// same ot BaseEnemy c'tor
	BaseBoss(const WwdObject& obj, Player* player,
		int damage, string walkAni, string hit1,
		string hit2, string fallDead, string strikeAni,
		string shootAni, string projectileAniDir);
	~BaseBoss();

	virtual void Logic(uint32_t elapsedTime) override = 0;

protected:
	bool checkForHurts() override; // We added this function because bosses are not hit by CC projectiles

	int32_t _hitsCuonter; // count the times CC hit the boss
	bool _blockClaw, _canJump;

private:
	const D2D1_POINT_2L _gemPos;
};


//////////////////////////////
// Help classes for enemies //
//////////////////////////////

// This class is responsible for leaving the enemy from the screen
class DeadEnemy : public BaseDynamicPlaneObject
{
public:
	DeadEnemy(const WwdObject& obj, shared_ptr<Animation> deadAni);
	void Logic(uint32_t elapsedTime) override;
};

class BossGem : public Item
{
public:
	BossGem(const WwdObject& obj, Player* player);
	void Logic(uint32_t elapsedTime) override;

private:
	D2D1_POINT_2F _destination;
};
