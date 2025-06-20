#pragma once

#include "GameEngine/GameEngineFramework.hpp"

class Player;


class InvincibilityComponent
{
public:
	InvincibilityComponent(Player* player);

	void update(uint32_t elapsedTime);

	void reset() { currColorIdx = 0; _timeCounter = 0; }

	int calcNextColor() {
		currColorIdx = (currColorIdx + 1) % colors.size();
		return (int)currColorIdx;
	}
	const ColorF* getColor() const { return &(colors[currColorIdx]); }

	const vector<ColorF>* getColorsList() const { return &colors;  }

private:
	static const vector<ColorF> colors;

	Player* player;
	size_t currColorIdx;
	int _timeCounter; // timer for color changeing in milliseconds
};
