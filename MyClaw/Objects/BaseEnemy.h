#pragma once

#include "../BaseCharacter.h"
#include "Item.h"


class BaseEnemy : public BaseCharacter
{
public:
	BaseEnemy(const WwdObject& obj, Player* player,
		int8_t health, int8_t damage, string walkAni,
		string hit1, string hit2, string fallDead,
		string strikeAni, string shootAni, string shootDuckAni, string projectileAniDir,
		vector<pair<string, uint32_t>> standAnisData, bool noTreasures = false);
	// `standAnis` is list of { ani-name, ani-duration (ms) }

	~BaseEnemy();

	void Logic(uint32_t elapsedTime) override;

	bool isStanding() const override;
	bool isDuck() const override;
	bool isTakeDamage() const override;

	void stopMovingLeft(float collisionSize) override;
	void stopMovingRight(float collisionSize) override;

	vector<Item*> getItems();


protected:
	virtual void makeAttack();


	bool isWalkAnimation() const;
	bool checkForHurt(pair<D2D1_RECT_F, uint8_t> hurtData); // returns `true` if the enemy hurt. `hurtData`={rect,damage}
	bool checkForHurts();

	struct StandAniData
	{
		StandAniData(shared_ptr<Animation> ani, uint32_t duration);

		const shared_ptr<Animation> ani;
		const uint32_t duration; // milliseconds
		uint32_t elapsedTime;
	};

	const string _walkAni, _hit1, _hit2, _fallDead, _strikeAniName, _shootAniName, _shootDuckAniName, _projectileAniDir;
	vector<int8_t> _itemsTypes;
	vector<shared_ptr<StandAniData>> _standAni;
	D2D1_RECT_F _saveCurrRect;
	size_t _standAniIdx;
	const float _minX, _maxX;
	int8_t _damage; // the amount of health that enemy took when he hit Claw
	bool _itemsTaken; // store if the items were taken from this crate. TODO: rename
	bool _isStanding;
	const bool _canStrike, _canShoot, _canShootDuck;
	const bool _isStaticEnemy; // it always idle
};


class BaseBoss : public BaseEnemy
{
public:
	// same ot BaseEnemy c'tor
	BaseBoss(const WwdObject& obj, Player* player,
		int8_t health, int8_t damage, string walkAni,
		string hit1, string hit2, string strikeAni,
		string shootAni, string projectileAniDir,
		vector<pair<string, uint32_t>> standAnisData);
	// `standAnis` is list of { ani-name, ani-duration (ms) }
	~BaseBoss();

	void Logic(uint32_t elapsedTime) override;

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
