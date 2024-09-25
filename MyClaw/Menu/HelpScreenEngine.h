#pragma once

#include "ScreenEngine.h"


class HelpScreenEngine : public ScreenEngine
{
public:
	HelpScreenEngine(shared_ptr<ClawLevelEngineFields> clawLevelEngineFields);
	void OnKeyUp(int key) override;
	void OnMouseButtonUp(MouseButtons btn) override;
};
