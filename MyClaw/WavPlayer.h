#pragma once

#include "RezArchive.h"


class WavPlayer
{
public:
	WavPlayer(shared_ptr<BufferReader> wavReader);
	~WavPlayer();

	// start sound the WAV file
	void play(bool infinite);
	// stop sound and reset it
	void stop();

	// set the volume. value range is [0,100]
	void setVolume(int32_t volume);

	uint32_t getDuration() const;

	bool shouldPlay() const;
	bool isInfinite() const;


private:
	// writes the error message in the debuggers output window
	static void WavError(MMRESULT mmResult);
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
