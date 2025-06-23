#pragma once

#include "GameEngine/GameEngineFramework.hpp"
#include "GameEngine/Timer.h"

class Player;


class InvincibilityComponent
{
public:
	InvincibilityComponent(Player* player);

	void init(); // init to start timer and colors

	const ColorF* getColor() const { return &(colors[currColorIdx]); }

	const vector<ColorF>* getColorsList() const { return &colors;  }

private:
	void changePlayerColor();

	static const vector<ColorF> colors;

	Player* player;
	Timer timer;
	size_t currColorIdx;
};
