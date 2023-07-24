#pragma once

#include "BaseEnemy.h"
#include "Cannon.h"


class Rat : public BaseEnemy
{
public:
	Rat(const WwdObject& obj);

	void makeAttack() override;
	pair<Rectangle2D, int> GetAttackRect() override;

	bool isDuck() const override;
	bool isTakeDamage() const override;
};


// rat with cannon
class PunkRat : public BaseEnemy
{
public:
	PunkRat(const WwdObject& obj);
	~PunkRat();

	void Logic(uint32_t elapsedTime) override;
	void Draw() override;
	Rectangle2D GetRect() override;
	pair<Rectangle2D, int> GetAttackRect() override;

private:
	Cannon* _cannon;
};
