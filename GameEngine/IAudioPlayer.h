#pragma once

#include "Framework/Framework.h"


class IAudioPlayer
{
public:
	IAudioPlayer(const string& key, const vector<uint8_t>& soundData)
		: _key(key), _soundData(soundData), _duration(0), _isPlaying(false),
		_infinite(false), _tryPlaying(false) {}
	virtual ~IAudioPlayer() {}

	virtual void play(bool infinite) = 0; // start sound (recomended as async)
	virtual void stop() = 0; // stop sound and reset it
	virtual void reset() = 0; // reset the sound player
	virtual void setVolume(int volume) = 0; // set the volume. value range is [0,100]

	uint32_t getDuration() const { return _duration; }
	bool isPlaying() const { return _isPlaying; }
	bool isInfinite() const { return _infinite; }
	bool shouldPlay() const { return !_isPlaying && _infinite && !_tryPlaying; }

	void setKey(const string& key) { _key = key; }
	const string& getKey() const { return _key; }

protected:
	string _key;
	vector<uint8_t> _soundData;
	uint32_t _duration;
	bool _isPlaying, _infinite, _tryPlaying;
};
