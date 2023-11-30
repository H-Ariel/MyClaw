#pragma once

#include "ScreenEngine.h"
#include "../Objects/Item.h"


class LevelEndEngine : public ScreenEngine
{
public:
	LevelEndEngine(int lvlNum, const map<Item::Type, uint32_t>& collectedTreasures);

	void Logic(uint32_t elapsedTime) override;

	void OnKeyUp(int key) override;
	void OnMouseButtonUp(MouseButtons btn) override;

private:
	void playNextLevel();

	enum State : int8_t {
		Start,
		DrawScore,
		Wait,
		End
	};

	map<Item::Type, uint32_t> _collectedTreasures;
	const int _lvlNum;
	int8_t _state;
};
