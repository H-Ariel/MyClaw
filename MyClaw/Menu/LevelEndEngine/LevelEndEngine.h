#pragma once

#include "../ScreenEngine.h"
#include "LevelEngine/Objects/Item.h"


class LevelEndEngineState;

class LevelEndEngine : public ScreenEngine
{
public:
	LevelEndEngine(int lvlNum, const map<Item::Type, uint32_t>& collectedTreasures);
	~LevelEndEngine();

	void Logic(uint32_t elapsedTime) override;

	void OnKeyUp(int key) override;
	void OnMouseButtonUp(MouseButtons btn) override;

private:
	void nextState();

	map<Item::Type, uint32_t> _collectedTreasures;
	const int _lvlNum;
	LevelEndEngineState* _state, * _nextState;


	friend class DrawScoreState;
	friend class EndState;
};
