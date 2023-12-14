#pragma once

#include "../BaseEnemy.h"


class LordOmarShield;
class LordOmar : public BaseBoss
{
public:
	LordOmar(const WwdObject& obj);

	void Logic(uint32_t elapsedTime) override;

	Rectangle2D GetRect() override;
	pair<Rectangle2D, int> GetAttackRect() override;
	bool checkForHurts() override;

private:
	void advanceState();

	enum States : int8_t
	{
		FireShield_1,
		IceShield_1,
		FireShield_2,
		IceShield_2,
		Bullet_1,
		Bullet_2,
		End
	};

	LordOmarShield* _shield;
	int8_t _state;
	bool stateInited;
};
