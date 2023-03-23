#pragma once

#include "../BasePlaneObject.h"


class SoundObjectBase : public BaseStaticPlaneObject
{
public:
	SoundObjectBase(const WwdObject& obj, Player* player);

protected:
	const string _wavPath; // the sound file
	int32_t _volume;
	uint32_t _wavPlayerId;
};


class SoundTrigger : public SoundObjectBase
{
public:
	SoundTrigger(const WwdObject& obj, Player* player);
	void Logic(uint32_t elapsedTime) override;
	void Reset() override;

protected:
	const int32_t _timesToPlay;
	int32_t _timesCounter;

private:
	const bool _isClawDialog;
	bool _isInCollision;
};

class AmbientSound : public SoundObjectBase
{
public:
	AmbientSound(const WwdObject& obj, Player* player);
	void Logic(uint32_t elapsedTime) override;

private:
	bool _isPlaying;
};

class GlobalAmbientSound : public SoundObjectBase
{
public:
	GlobalAmbientSound(const WwdObject& obj, Player* player);
	void Logic(uint32_t elapsedTime) override;

private:
	const uint32_t _minTimeOn, _maxTimeOn; // milliseconds
	const uint32_t _minTimeOff, _maxTimeOff;
	int32_t _currentTime, _timeOff, _soundDurationMs;
	const bool _isLooping;
};
