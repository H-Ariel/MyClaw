#pragma once

#include "BaseEnemy.h"


class Seagull : public BaseEnemy
{
public:
	Seagull(const WwdObject& obj, Player* player);

	void Logic(uint32_t elapsedTime) override;
	pair<Rectangle2D, int> GetAttackRect() override;

private:
	enum class States : int8_t { Fly, DiveIn, DiveOut };
	const float _minY, _maxY;
	States _state;
};
