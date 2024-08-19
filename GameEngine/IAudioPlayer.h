#pragma once

#include "Framework/Framework.h"


class IAudioPlayer
{
public:
	IAudioPlayer(const string& key, const vector<uint8_t>& soundData)
		: _key(key), _soundData(soundData), _isPlaying(false) {}
	virtual ~IAudioPlayer() {}

	virtual void play(bool infinite) = 0; // start sound (recomended as async)
	virtual void stop() = 0; // stop sound and reset it
	virtual void reset() = 0; // reset the sound player
	virtual void setVolume(int volume) = 0; // set the volume. value range is [0,100]

	bool isPlaying() const { return _isPlaying; }
	const string& getKey() const { return _key; }

protected:
	const string _key;
	vector<uint8_t> _soundData;
	bool _isPlaying;
};
