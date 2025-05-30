#pragma once

#include "../Objects/BaseCharacter.h"


constexpr float ENEMY_PATROL_SPEED = 0.1f;


class BaseEnemy : public BaseCharacter
{
public:
	BaseEnemy(const WwdObject& obj, int health, int damage,
		const string& walkAni, const string& hithigh, const string& hitlow, const string& fallDeadAni,
		const string& strikeAni, const string& strikeDuckAni, const string& shootAni,
		const string& shootDuckAni, const string& projectileAniDir,
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
	virtual bool PreLogic(uint32_t elapsedTime); // called before `Logic`. returns `true` if we need do `Logic`
	virtual void PostLogic(uint32_t elapsedTime); // called after `Logic`
	virtual void makeAttack(float deltaX, float deltaY); // recieves the distance between the enemy and CC
	virtual bool checkForHurts() override; // returns `true` if the enemy hurt (by CC attack, projectiles, etc.)

	void makeAttack(); // calc deltaX and deltaY and call `makeAttack(float deltaX, float deltaY)`
	bool doesEnemySeeClaw() const;
	bool isWalkAnimation() const;
	bool checkForHurt(const pair<Rectangle2D, int>& hurtData); // returns `true` if the enemy hurt. `hurtData`={rect,damage}
	bool checkClawHit(); // returns `true` if the enemy hurt by CC

	const string _projectileAniDir;

	shared_ptr<UIAnimation> _aniWalk, _aniHitHigh, _aniHitLow, _aniFallDead,
		_aniStrike, _aniStrikeDuck, _aniShoot, _aniShootDuck, _aniIdle;

	vector<int8_t> _itemsTypes;
	const float _minX, _maxX;
	const int _damage; // the amount of health that enemy took when he hit Claw
	int _attackRest; // rest time between attack. NOTE: not all enemies used that
	bool _isStanding;
	bool _fallDead; // `true` if enemy is falling dead
	const bool _canStrike, _canStrikeDuck, _canShoot, _canShootDuck;
	const bool _isStaticEnemy; // it always idle

	enum class DeathType : int8_t
	{
		Regular,
		FireSword,
		IceSword,
		LightningSword
	};
	DeathType _deathType;
};


class BaseBoss : public BaseEnemy
{
public:
	BaseBoss(const WwdObject& obj, const string& fallDeadAni = "");
	~BaseBoss();

	virtual void Logic(uint32_t elapsedTime) override = 0;

protected:
	virtual bool checkForHurts() override = 0; // We added this function because bosses are not hit by CC projectiles

	int _hitsCuonter; // count the times CC hit the boss
	bool _blockClaw, _canJump;

private:
	const D2D1_POINT_2L _gemPos;
};
