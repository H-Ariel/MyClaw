#pragma once

#include "../BasePlaneObject.h"
#include "UITextElement.h"


// TODO: create method of `ActionPlane` that creates `ActionPlaneMessage`s
class ActionPlaneMessage : public BasePlaneObject
{
public:
	ActionPlaneMessage(const wstring& message, int timeout = 2000);

	void Logic(uint32_t elapsedTime) override;
	void Draw() override;
	Rectangle2D GetRect() override;


	UITextElement text;

private:
	int _timeLeft;
};
