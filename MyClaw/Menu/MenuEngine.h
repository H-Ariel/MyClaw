#pragma once

#include "Assets-Managers/Animation.h"
#include "HierarchicalMenu.h"
#include "ScreenEngine.h"


class MenuEngine : public ScreenEngine
{
public:
	MenuEngine(const string& bgPcxPath = "");
	MenuEngine(D2D1_POINT_2U mousePosition, shared_ptr<Animation> cursor, const string& bgPcxPath = "");
	MenuEngine(shared_ptr<ClawLevelEngineFields> fields, const string& bgPcxPath = "");
	~MenuEngine();

	void Logic(uint32_t elapsedTime) override;
	void OnKeyUp(int key) override;

protected:
	static stack<const HierarchicalMenu*> _menusStack;
	static const HierarchicalMenu* _currMenu;

	void menuIn(const HierarchicalMenu* newMenu);
	void menuOut();
	void backToGame();

	shared_ptr<Animation> _cursor;

	friend class ScreenEngine;
};
