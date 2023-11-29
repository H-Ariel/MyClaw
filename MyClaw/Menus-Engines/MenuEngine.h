#pragma once

#include "../BaseEngine.h"
#include "../GUI/Animation.h"
#include "MenuBackgroundImage.h"
#include "HierarchicalMenu.h"


struct ClawLevelEngineFields;


class MenuEngine : public BaseEngine
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
	virtual void backToMenu();

	static stack<const HierarchicalMenu*> _menusStack;
	static const HierarchicalMenu* _currMenu;
	static shared_ptr<ClawLevelEngineFields> _clawLevelEngineFields;

	shared_ptr<Animation> _cursor;
	MenuBackgroundImage* _bgImg;
};
