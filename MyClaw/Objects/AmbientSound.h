#pragma once

#include "../BasePlaneObject.h"


class AmbientSound : public BaseSoundObject
{
public:
	AmbientSound(const WwdObject& obj);
	void Logic(uint32_t elapsedTime) override;

private:
	bool _isPlaying;
};


class GlobalAmbientSound : public BaseSoundObject
{
public:
	GlobalAmbientSound(const WwdObject& obj, int levelNumber);
	void Logic(uint32_t elapsedTime) override;

private:
	const uint32_t _minTimeOn, _maxTimeOn; // milliseconds
	const uint32_t _minTimeOff, _maxTimeOff;
	int32_t _currentTime, _timeOff, _soundDurationMs;
	const bool _isLooping;
};
