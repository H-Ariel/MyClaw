#pragma once

#include "BaseCharacter.h"
#include "Objects/Item.h"


#define ENEMY_PATROL_SPEED 0.1f


class BaseEnemy : public BaseCharacter
{
public:
	BaseEnemy(const WwdObject& obj, int health, int damage,
		string walkAni, string hit1, string hit2, string fallDead,
		string strikeAni, string strikeDuckAni, string shootAni,
		string shootDuckAni, string projectileAniDir,
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
	virtual void makeAttack(float deltaX, float deltaY); // recieves the distance between the enemy and CC
	void makeAttack(); // calc deltaX and deltaY and call `makeAttack(float deltaX, float deltaY)`

	virtual bool PreLogic(uint32_t elapsedTime); // called before `Logic`. returns `true` if we need do `Logic`
	virtual void PostLogic(uint32_t elapsedTime); // called after `Logic`


	bool isWalkAnimation() const;
	bool checkForHurt(pair<Rectangle2D, int> hurtData); // returns `true` if the enemy hurt. `hurtData`={rect,damage}
	virtual bool checkForHurts();

	bool enemySeeClaw() const;

	const string _walkAniName, _hit1AniName, _hit2AniName,
		_fallDeadAniName, _strikeAniName, _strikeDuckAniName,
		_shootAniName, _shootDuckAniName, _projectileAniDir, _idleAniName;
	vector<int8_t> _itemsTypes;
	const float _minX, _maxX;
	const int _damage; // the amount of health that enemy took when he hit Claw
	int _attackRest; // rest time between attack. NOTE: not all enemies used that
	bool _isStanding;
	bool _fallDead; // true if enemy is falling dead
	const bool _canStrike, _canStrikeDuck, _canShoot, _canShootDuck;
	const bool _isStaticEnemy; // it always idle
};


class BaseBoss : public BaseEnemy
{
public:
	// same ot BaseEnemy c'tor
	BaseBoss(const WwdObject& obj, int damage, string walkAni,
		string hit1, string hit2, string fallDead, string strikeAni,
		string shootAni, string projectileAniDir);
	~BaseBoss();

	virtual void Logic(uint32_t elapsedTime) override = 0;

protected:
	virtual bool checkForHurts() override = 0; // We added this function because bosses are not hit by CC projectiles

	int32_t _hitsCuonter; // count the times CC hit the boss
	bool _blockClaw, _canJump;

private:
	const D2D1_POINT_2L _gemPos;
};
