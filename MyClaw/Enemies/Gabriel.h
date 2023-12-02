#pragma once

#include "../BaseEnemy.h"


class Gabriel : public BaseBoss
{
public:
	Gabriel(const WwdObject& obj);
	void Logic(uint32_t elapsedTime) override;
	pair<Rectangle2D, int> GetAttackRect() override;

private:
	bool checkForHurts() override;

	int32_t _throwBombsTime, _sendPiratesTime;
	bool _canThrowBombs, _canSendPirates;
};


class GabrielCannon : public BaseStaticPlaneObject
{
public:
	GabrielCannon(const WwdObject& obj);
	void Logic(uint32_t elapsedTime) override;

private:
	shared_ptr<Animation> _home, _rest, _rise, _horzfire, _vertfire;
};


class GabrielCannonSwitch : public BaseStaticPlaneObject
{
public:
	GabrielCannonSwitch(const WwdObject& obj);
	void Logic(uint32_t elapsedTime) override;

private:
	int32_t _switchCannonTime;
};


class GabrielCannonButton : public BaseStaticPlaneObject
{
public:
	GabrielCannonButton(const WwdObject& obj);
	void Logic(uint32_t elapsedTime) override;

private:
	shared_ptr<Animation> _idle, _pressed;
};


// a pirate that Gabriel sends
class GabrielRedTailPirate : public BaseDynamicPlaneObject
{
public:
	GabrielRedTailPirate();
	void Logic(uint32_t elapsedTime) override;
	void stopFalling(float collisionSize) override;

private:
	bool _isJumping;
};
