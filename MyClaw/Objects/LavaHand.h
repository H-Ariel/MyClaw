#pragma once

#include "BasePlaneObject.h"
#include "GameEngine/UITextElement.h"


class LavaHand : public BaseStaticPlaneObject
{
public:
	LavaHand(const WwdObject& obj);

	void Logic(uint32_t elapsedTime) override;

private:
	const int _delay; // delay between two throws
	bool _throwed;
};
