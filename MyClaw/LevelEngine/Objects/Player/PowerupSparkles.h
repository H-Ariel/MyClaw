#pragma once

#include "GameEngine/UIAnimation.h"


class PowerupSparkles
{
public:
	PowerupSparkles(Rectangle2D* playerRc);
	void Logic(uint32_t elapsedTime);
	void Draw();

private:
	void init(shared_ptr<UIAnimation> sparkle); // init one sparkle (from the list)

	Rectangle2D* _playerRc;
	shared_ptr<UIAnimation> _sparkles[30]; // all sparkles' list
};
