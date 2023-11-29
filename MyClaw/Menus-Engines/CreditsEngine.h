#pragma once

#include "MenuEngine.h"
#include "../GUI/UITextElement.h"


class CreditsEngine : public MenuEngine
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
	const float _startY, _endY;
};
