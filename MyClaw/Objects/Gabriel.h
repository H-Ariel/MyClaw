#pragma once

#include "BaseEnemy.h"


class Gabriel : public BaseBoss
{
public:
	Gabriel(const WwdObject& obj, Player* player);

	void Logic(uint32_t elapsedTime) override;

	pair<Rectangle2D, int8_t> GetAttackRect() override;

private:
	void makeAttack() override;
	bool checkForHurts() override;

	int32_t _switchCannonTime;
};

class GabrielCannon : public BaseStaticPlaneObject
{
public:
	GabrielCannon(const WwdObject& obj, Player* player);

	void Logic(uint32_t elapsedTime) override;

private:
	shared_ptr<Animation> _home, _rest, _rise, _horzfire, _vertfire;
};

class CannonSwitch : public BaseStaticPlaneObject
{
public:
	CannonSwitch(const WwdObject& obj, Player* player);
	void Logic(uint32_t elapsedTime) override;
};

class CannonButton : public BaseStaticPlaneObject
{
public:
	CannonButton(const WwdObject& obj, Player* player);
	void Logic(uint32_t elapsedTime) override;

private:
	shared_ptr<Animation> _idle, _pressed;
	int32_t _pressedTime;
};
