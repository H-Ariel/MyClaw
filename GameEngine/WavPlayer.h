#pragma once

#include "IAudioPlayer.h"


class WavPlayer : public IAudioPlayer
{
public:
	WavPlayer(const string& key, const WAVEFORMATEX& fmt, const vector<uint8_t>& wavSoundData);
	~WavPlayer() override;

	void play(bool infinite) override; // start sound the WAV file
	void stop() override; // stop sound and reset it
	void reset() override {}
	void setVolume(int volume) override; // set the volume. value range is [0,100]


private:
	static void CALLBACK waveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2);


	const WAVEFORMATEX _fmt;
	WAVEHDR _hdr;
	HWAVEOUT _wav;
	DWORD _volume; // low-word contains the left-channel volume setting, and the high-word contains the right-channel setting
	bool _tryPlaying, _infinite;
};
