#pragma once

#include "RezArchive.h"


class WavPlayer
{
public:
	WavPlayer(WAVEFORMATEX fmt, vector<uint8_t> wavSoundData, uint32_t soundDataLength);
	~WavPlayer();

	// start sound the WAV file
	void play(bool infinite);
	// stop sound and reset it
	void stop();

	// set the volume. value range is [0,100]
	void setVolume(int volume);

	uint32_t getDuration() const { return _duration; }
	bool shouldPlay() const { return !_isPlaying && _infinite && !_tryPlaying; }
	bool isInfinite() const { return _infinite; }


private:
	static void CALLBACK waveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2);


	vector<uint8_t> _wavSoundData;
	const WAVEFORMATEX _fmt;
	WAVEHDR _hdr;
	HWAVEOUT _wav;
	DWORD _volume; // low-word contains the left-channel volume setting, and the high-word contains the right-channel setting
	uint32_t _duration;
	bool _infinite;
	bool _isPlaying, _tryPlaying;
};
