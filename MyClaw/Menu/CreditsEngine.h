#pragma once

#include "ScreenEngine.h"
#include "GameEngine/UITextElement.h"


class CreditsEngine : public ScreenEngine
{
public:
	CreditsEngine();
	~CreditsEngine();

	void Logic(uint32_t elapsedTime) override;
	void OnKeyUp(int key) override;
	void OnMouseButtonUp(MouseButtons btn) override;
	void OnResize() override;

private:
	UITextElement* _creditsTextElement;
	float _startY, _endY;
};
