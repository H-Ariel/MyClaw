#pragma once

#include "../BaseEnemy.h"


class LordOmarShield;
class LordOmar : public BaseBoss
{
public:
	LordOmar(const WwdObject& obj);
	~LordOmar();

	void Logic(uint32_t elapsedTime) override;

	Rectangle2D GetRect() override;
	pair<Rectangle2D, int> GetAttackRect() override;

private:
	bool checkForHurts() override;
	void advanceState();

	enum States : int8_t
	{
		Shield_1,
		Shield_2,
		Shield_3,
		Shield_4,
		Bullet_1,
		Bullet_2,
		Bullet_3,
		Bullet_4,
		End
	};

	LordOmarShield* _shield;
	int8_t _state;
	bool stateInited;
};
