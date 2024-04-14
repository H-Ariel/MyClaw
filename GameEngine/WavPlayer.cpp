#include "WavPlayer.h"


#define WAV_VOLUME_MAX 0xFFFF

#ifdef _DEBUG
// writes the error message in the debuggers output window
static void WAV_CALL(MMRESULT mmResult)
{
	if (mmResult != MMSYSERR_NOERROR)
	{
		char text[256];
		waveOutGetErrorTextA(mmResult, text, sizeof(text));
		DBG_PRINT("WaveError: %s\n", text);
	}
}
#else
#define WAV_CALL(func) func
#endif


WavPlayer::WavPlayer(const string& key, const WAVEFORMATEX& fmt, const vector<uint8_t>& wavSoundData)
	: IAudioPlayer(key, wavSoundData), _wav(nullptr), _fmt(fmt), _hdr({}),
	_volume(make_dword(WAV_VOLUME_MAX, WAV_VOLUME_MAX))
{
	_duration = (uint32_t)((float)wavSoundData.size() / fmt.nAvgBytesPerSec * 1000); // duartion in milliseconds
}
WavPlayer::~WavPlayer()
{
	stop();
}

void WavPlayer::play(bool infinite)
{
	stop();
	
	thread([&](bool infinite) {
		_tryPlaying = true;
		WAV_CALL(waveOutOpen(&_wav, WAVE_MAPPER, &_fmt, (DWORD_PTR)waveOutProc, (DWORD_PTR)this, CALLBACK_FUNCTION));
		if (_wav)
		{
			WAV_CALL(waveOutPrepareHeader(_wav, &_hdr, sizeof(_hdr)));
			_hdr.lpData = (LPSTR)_soundData.data();
			_hdr.dwBufferLength = (DWORD)_soundData.size();
			WAV_CALL(waveOutSetVolume(_wav, _volume));
			WAV_CALL(waveOutWrite(_wav, &_hdr, sizeof(_hdr)));
			_isPlaying = true;
			_tryPlaying = false;
			_infinite = infinite;
		}
	}, infinite).detach();
}

void WavPlayer::stop()
{
	try
	{
		if (_wav)
		{
			WAV_CALL(waveOutReset(_wav));
			WAV_CALL(waveOutUnprepareHeader(_wav, &_hdr, sizeof(_hdr)));
			WAV_CALL(waveOutClose(_wav));
		}
	}
	catch (...) {}

	_wav = nullptr;
	_hdr = {};
	_isPlaying = false;
}

void WavPlayer::setVolume(int volume)
{
	WORD tmp = (WORD)(volume / 150.f * WAV_VOLUME_MAX);
	_volume = make_dword(tmp, tmp); // the right and left channel get the same volume
	
	/*if (_wav)
	{
		WAV_CALL(waveOutSetVolume(_wav, _volume));
	}*/
}

void WavPlayer::waveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
	if (uMsg == WOM_DONE)
	{
		if (dwInstance)
		{
			((WavPlayer*)dwInstance)->_isPlaying = false;
		}
	}
}
