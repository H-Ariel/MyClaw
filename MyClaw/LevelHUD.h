#pragma once

#include "Objects/Player.h"


class ActionPlane;


// Level Head-Up Display
class LevelHUD : public UIBaseElement
{
public:
	LevelHUD(ActionPlane* actionPlane);

	void Draw() override;


private:
	void drawNumbers(uint32_t amount, int numOfDigits, shared_ptr<UIBaseImage> const numArr[], float posX, float posY) const;

	shared_ptr<UIAnimation> _chest, _health, _lives, _stopwatch;
	shared_ptr<UIBaseImage> _bossBar;
	map<ClawProjectile::Types, shared_ptr<UIAnimation>> _weaponAni;
	shared_ptr<UIBaseImage> _healthNumbers[10], _smallNumbers[10], _scoreNumbers[10];
	ActionPlane* _actionPlane;
	int _bossHealth;
};
