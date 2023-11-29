#pragma once

#include "MenuEngine.h"


class OpeningScreenEngine : public MenuEngine
{
public:
	OpeningScreenEngine();
	void Logic(uint32_t elapsedTime) override;
	void OnKeyUp(int key) override;
	void OnMouseButtonUp(MouseButtons btn) override;

private:
	void continueToMenu();

	uint32_t _wavId;
	int _totalTime;
};
