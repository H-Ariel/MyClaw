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

	bool isIdleAni() const { return _ani == _animations.at("IDLE1") || _ani == _animations.at("IDLE2"); }
	void setIdleAni() {
		_ani = getRandomInt(0, 1) == 0
			? _animations["IDLE1"] : _animations["IDLE2"];
	}

	int32_t _throwBombsTime;
	int32_t _sendPiratesTime;
	bool _canThrowBomb, _canSendPirates;
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

private:
	int32_t _switchCannonTime;
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
