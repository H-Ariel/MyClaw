#pragma once

#include "../BasePlaneObject.h"


class SoundTrigger : public BaseSoundObject
{
public:
	SoundTrigger(const WwdObject& obj);
	void Logic(uint32_t elapsedTime) override;
	void Reset() override;

private:
	const int32_t _timesToPlay;
	int32_t _timesCounter;
	const bool _isClawDialog;
	bool _isInCollision;
};
