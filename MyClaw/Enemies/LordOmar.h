#pragma once

#include "../BaseEnemy.h"


class LordOmar : public BaseBoss
{
public:
	LordOmar(const WwdObject& obj);
	~LordOmar();

	void Logic(uint32_t elapsedTime) override;

	Rectangle2D GetRect() override;
	pair<Rectangle2D, int> GetAttackRect() override;
	bool checkForHurts() override;

private:
	enum class States
	{
		FireShield_1,
		IceShield_1,
		FireShield_2,
		IceShield_2,
		Bullet_1,
		Bullet_2,
		End
	};

	States _state;
};
