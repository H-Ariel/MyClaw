#pragma once

#include "BaseEnemy.h"
#include "../Objects/Cannons/Cannon.h"


// Rat with cannon
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
