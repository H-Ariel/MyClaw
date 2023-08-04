#pragma once

#include "../BasePlaneObject.h"
#include "../Objects/Stalactite.h"


class Tentacle : public BaseDamageObject
{
public:
	Tentacle(const WwdObject& obj);
	void Logic(uint32_t elapsedTime) override;
	Rectangle2D GetRect() override;
	bool isDamage() const override;

private:
	bool checkForHurts();

	shared_ptr<Animation> _idle, _hit, _killfall, _respawn,
		_slap, // tentacle slap/whip (when player is too far)
		_squeeze; // tentacle squeeze (when player is too close)
	Rectangle2D _lastAttackRect;
};


class AquatisCrack : public BaseStaticPlaneObject
{
public:
	AquatisCrack(const WwdObject& obj);

	void Logic(uint32_t elapsedTime) override;
};


class AquatisDynamite : public BaseStaticPlaneObject
{
public:
	AquatisDynamite(const WwdObject& obj);

	void Logic(uint32_t elapsedTime) override;

private:
	int _timeCounter;
};


class AquatisStalactite : public Stalactite
{
public:
	AquatisStalactite(const WwdObject& obj);
	~AquatisStalactite();

	void Logic(uint32_t elapsedTime) override;
	int getDamage() const override;

};
