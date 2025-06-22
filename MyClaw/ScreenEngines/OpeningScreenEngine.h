#pragma once

#include "ScreenEngine.h"


class OpeningScreenEngine : public ScreenEngine
{
public:
	OpeningScreenEngine();
	~OpeningScreenEngine();
	void OnKeyUp(int key) override;
	void OnMouseButtonUp(MouseButtons btn) override;

private:
	void continueToMenu();

	uint32_t _wavId;
	Timer* timer;
};
