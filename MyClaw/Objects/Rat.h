#pragma once

#include "BaseEnemy.h"
#include "Cannon.h"


class Rat : public BaseEnemy
{
public:
	Rat(const WwdObject& obj, Player* player);

	void makeAttack() override;
	D2D1_RECT_F GetRect() override;
	pair<D2D1_RECT_F, int8_t> GetAttackRect() override;

	bool isDuck() const override;
	bool isTakeDamage() const override;
};


// rat with cannon
class PunkRat : public BaseEnemy
{
public:
	PunkRat(const WwdObject& obj, Player* player);
	~PunkRat();

	void Logic(uint32_t elapsedTime) override;
	void Draw() override;
	D2D1_RECT_F GetRect() override;
	pair<D2D1_RECT_F, int8_t> GetAttackRect() override;

private:
	Cannon* _cannon;
};
