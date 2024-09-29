#pragma once

#include "ScreenEngine.h"


// TODO: try not use this class (we have HirarchicalMenu::HelpScreen)
class HelpScreenEngine : public ScreenEngine
{
public:
	HelpScreenEngine(shared_ptr<ClawLevelEngineFields> clawLevelEngineFields);
	void OnKeyUp(int key) override;
	void OnMouseButtonUp(MouseButtons btn) override;
};
