#pragma once

#include "../BasePlaneObject.h"
#include "UITextElement.h"


class ActionPlaneMessage : public BasePlaneObject
{
public:
	ActionPlaneMessage(const wstring& message, int timeout);

	void Logic(uint32_t elapsedTime) override;
	void Draw() override;
	Rectangle2D GetRect() override;


	UITextElement text;

private:
	int _timeLeft;
};