#pragma once

#include "ActionPlane.h"


class LevelMap : public UIBaseElement
{
public:
	LevelMap(int8_t levelNumber);
	~LevelMap() override;

	void Logic(uint32_t elapsedTime) override;
	void Draw() override;

	Player* getPlayer() const { return _actionPlane->getPlayer(); }
	const D2D1_POINT_2F* getWindowOffset() const { return &_actionPlane->position; }


private:
	vector<LevelPlane*> _planes;
	shared_ptr<WapWorld> _wwd;
	ActionPlane* _actionPlane;
};
