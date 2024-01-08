#pragma once

#include "../BasePlaneObject.h"
#include "GameEngine/UITextElement.h"


class ActionPlaneMessage : public BasePlaneObject
{
public:
	ActionPlaneMessage(const string& message, int timeout = 2000);

	void Logic(uint32_t elapsedTime) override;
	void Draw() override;
	Rectangle2D GetRect() override;


	UITextElement text;

private:
	int _timeLeft;
};
