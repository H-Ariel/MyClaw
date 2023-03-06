#pragma once

#include "ActionPlane.h"


class ClawLevelEngine;
class MidiPlayer;


class LevelMap : public UIBaseElement
{
public:
	LevelMap(ClawLevelEngine* engine, uint8_t levelNumber);
	~LevelMap() override;

	void Logic(uint32_t elapsedTime) override;
	void Draw() override;

	Player* getPlayer();
	const D2D1_POINT_2F* getWindowOffset();


private:
	vector<LevelPlane*> _planes;
	ActionPlane* _actionPlane;
	shared_ptr<WapWorld> _wwd;
};
