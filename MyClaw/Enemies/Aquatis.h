#pragma once

#include "../BaseEnemy.h"
#include "../Objects/Projectile.h"


class Aquatis : public BaseBoss
{
public:
	Aquatis(const WwdObject& obj);
	~Aquatis();

	void Logic(uint32_t elapsedTime) override;
	pair<Rectangle2D, int> GetAttackRect() override;
	bool checkForHurts() override;
};


class AquatisTentacle : public BaseDamageObject
{
public:
	AquatisTentacle(const WwdObject& obj);
	~AquatisTentacle();
	void Logic(uint32_t elapsedTime) override;
	Rectangle2D GetRect() override;
	bool isDamage() const override;

private:
	bool checkForHurts();

	shared_ptr<Animation> _idle, _hit, _killfall, _respawn,
		_slap, // tentacle slap/whip (when player is too far)
		_squeeze; // tentacle squeeze (when player is too close)
	Rectangle2D _lastAttackRect;
	int _squeezeRestTime;
	int _deadTime;
};


class AquatisCrack : public BaseStaticPlaneObject
{
public:
	AquatisCrack(const WwdObject& obj);

	void Logic(uint32_t elapsedTime) override;

private:
	Projectile* _lastDynamite;
};


class AquatisDynamite : public BaseStaticPlaneObject
{
public:
	AquatisDynamite(const WwdObject& obj);

	void Logic(uint32_t elapsedTime) override;

	static void playerTookDynamite() { _playerTookDynamite = true; }

private:
	int _timeCounter;
	static bool _playerTookDynamite; // flag to know if the player took the dynamite so he doesn't get 2 dynamites at the same time
};


class AquatisStalactite : public Projectile
{
public:
	AquatisStalactite(const WwdObject& obj);
	~AquatisStalactite();

	void Logic(uint32_t elapsedTime) override;
	void stopFalling(float collisionSize) override;
	int getDamage() const override;

private:
	const int _idx;
};
