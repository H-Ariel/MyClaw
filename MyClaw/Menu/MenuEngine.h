#pragma once

#include "../GUI/Animation.h"
#include "HierarchicalMenu.h"
#include "ScreenEngine.h"


class MenuEngine : public ScreenEngine
{
public:
	MenuEngine(); // default background image with children
	MenuEngine(const string& bgPcxPath); // just image without children
	MenuEngine(bool allocChildren, const string& bgPcxPath);
	MenuEngine(D2D1_POINT_2U mousePosition, shared_ptr<Animation> cursor, bool allocChildren = true, const string& bgPcxPath = "");
	MenuEngine(shared_ptr<ClawLevelEngineFields> clawLevelEngineFields, bool allocChildren = true, const string& bgPcxPath = "");
	~MenuEngine();

	void Logic(uint32_t elapsedTime) override;

protected:
	static stack<const HierarchicalMenu*> _menusStack;
	static const HierarchicalMenu* _currMenu;

	shared_ptr<Animation> _cursor;

	friend class ScreenEngine;
};
