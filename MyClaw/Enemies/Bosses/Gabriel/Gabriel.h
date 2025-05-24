#pragma once

#include "../../BaseEnemy.h"


class Gabriel : public BaseBoss
{
public:
	Gabriel(const WwdObject& obj);
	~Gabriel();
	void Logic(uint32_t elapsedTime) override;
	pair<Rectangle2D, int> GetAttackRect() override;

	void makeHurt();
	void changeSwitch();

private:
	bool checkForHurts() override;

	int _throwBombsTime, _sendPiratesTime;
	bool _canThrowBombs, _canSendPirates;
};


class GabrielCannon : public BaseStaticPlaneObject
{
public:
	GabrielCannon(const WwdObject& obj);
	~GabrielCannon();
	void Logic(uint32_t elapsedTime) override;

	bool isUp() const { return _ani == _rise; }
	void operateCannon();
	void riseCannon();

private:
	void setAnimation(shared_ptr<UIAnimation> newAni);

	shared_ptr<UIAnimation> _home, _rest, _rise, _horzfire, _vertfire;
};


class GabrielCannonSwitch : public BaseStaticPlaneObject
{
public:
	GabrielCannonSwitch(const WwdObject& obj);
	void Logic(uint32_t elapsedTime) override;

private:
	int _switchCannonTime;
};


class GabrielCannonButton : public BaseStaticPlaneObject
{
public:
	GabrielCannonButton(const WwdObject& obj);
	void Logic(uint32_t elapsedTime) override;

private:
	shared_ptr<UIAnimation> _idle, _pressed;
};
