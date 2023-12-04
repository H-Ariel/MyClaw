#include "WavPlayer.h"


#define WAV_CALL(func) { MMRESULT mmResult; if ((mmResult = func) != MMSYSERR_NOERROR) WavError(mmResult); }
#define WAV_VOLUME_MAX 0xFFFF


inline DWORD make_dword(WORD hi, WORD lo)
{
	return ((DWORD)hi << 16) | (DWORD)lo;
}


WavPlayer::WavPlayer(shared_ptr<BufferReader> wavReader)
	: _wav(nullptr), _fmt({}), _hdr({}), _infinite(false),
	_isPlaying(false), _tryPlaying(false),
	_volume(make_dword(WAV_VOLUME_MAX, WAV_VOLUME_MAX))
{
	WAVEFORMATEX fmt = {};

	uint32_t soundDataLength;
	fmt.cbSize = sizeof(WAVEFORMATEX);
	wavReader->skip(20);
	wavReader->read(fmt.wFormatTag);
	wavReader->read(fmt.nChannels);
	wavReader->read(fmt.nSamplesPerSec);
	wavReader->read(fmt.nAvgBytesPerSec);
	wavReader->read(fmt.nBlockAlign);
	wavReader->read(fmt.wBitsPerSample);
	wavReader->skip(4);
	wavReader->read(soundDataLength);
	_wavSoundData = wavReader->ReadVector(soundDataLength, true);

	myMemCpy(_fmt, fmt);
	_duration = soundDataLength / fmt.nAvgBytesPerSec * 1000; // duartion in milliseconds
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
			_hdr.lpData = (LPSTR)_wavSoundData.data();
			_hdr.dwBufferLength = (DWORD)_wavSoundData.size();
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

void WavPlayer::setVolume(int32_t volume)
{
	WORD tmp = (WORD)(volume / 150.f * WAV_VOLUME_MAX);
	_volume = make_dword(tmp, tmp); // the right and left channel get the same volume
	
	/*if (_wav)
	{
		WAV_CALL(waveOutSetVolume(_wav, _volume));
	}*/
}

void WavPlayer::WavError(MMRESULT mmResult)
{
#ifdef _DEBUG
	char text[512];
	waveOutGetErrorTextA(mmResult, text, sizeof(text));
	DBG_PRINT("WaveError: %s\n", text);
#endif
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
