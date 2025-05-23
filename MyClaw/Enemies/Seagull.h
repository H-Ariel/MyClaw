#pragma once

#include "BaseEnemy.h"


class Seagull : public BaseEnemy
{
public:
	Seagull(const WwdObject& obj);

	void Logic(uint32_t elapsedTime) override;
	pair<Rectangle2D, int> GetAttackRect() override;

private:
	const float _minY, _maxY;
};
