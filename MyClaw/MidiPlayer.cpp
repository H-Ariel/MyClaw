#include "MidiPlayer.h"


const LARGE_INTEGER EMPTY_LARGE_INTEGER = {};
static LARGE_INTEGER PerformanceFrequency = {};

vector<uint8_t> xmiToMidi(vector<uint8_t> xmiFileData); // impleted i "XmiFile.cpp"


MidiPlayer::MidiPlayer(vector<uint8_t> xmiFileData)
	: _soundData(xmiToMidi(xmiFileData)), _midiOut(nullptr), _isPlaying(false)
{
	if (!memcmp(&PerformanceFrequency, &EMPTY_LARGE_INTEGER, sizeof(LARGE_INTEGER)))
	{
		QueryPerformanceFrequency(&PerformanceFrequency);
	}

	reset();
}
MidiPlayer::~MidiPlayer()
{
	stop();
}

void MidiPlayer::play(bool infinite)
{
	_isPlaying = true;
	thread t(&MidiPlayer::playSync, this, infinite);
	t.detach();
}

void MidiPlayer::stop()
{
	_mutex.lock();
	_isPlaying = false;
	midiOutReset(_midiOut);
	midiOutClose(_midiOut);
	_midiOut = nullptr;
	_mutex.unlock();
}

void MidiPlayer::playSync(bool infinite)
{
	// TODO: sometimes when we play again (after we stop the player) the sound is broken :(
	
	if (midiOutOpen(&_midiOut, MIDI_MAPPER, 0, 0, CALLBACK_NULL) != MMSYSERR_NOERROR)
	{
		return;
	}

	for (uint32_t& m : _cmdMsgs)
	{
		midiOutShortMsg(_midiOut, m);
	}

	MidiEvent evt;
	uint32_t time, bytesRead, msg;
	uint8_t len, meta;

	while (_state != PlaySyncState::EndPlay && _isPlaying)
	{
		_mutex.lock();

		time = -1;
		bytesRead = 0;

		_state = PlaySyncState::EndPlay;

		evt = getNextEvent();
		if ((evt.event != 0xff || evt.data[1] != 0x2f) && evt.absoluteTime <= time)
		{
			time = evt.absoluteTime;
			_state = PlaySyncState::NeedPlay;
		}
		else if (infinite)
		{
			reset();
			_isPlaying = true;
			_mutex.unlock();
			continue;
		}
		else
		{
			_isPlaying = false;
			_mutex.unlock();
			break;
		}

		_track.absoluteTime = evt.absoluteTime;
		time = _track.absoluteTime - _currentTime;
		_currentTime = _track.absoluteTime;

		_mutex.unlock();
		usleep(time * _ppqnClock);
		_mutex.lock();
		if (_state == PlaySyncState::EndPlay || !_isPlaying)
		{
			_mutex.unlock();
			break;
		}

		if ((evt.event & 0x80) == 0) // running mode
		{
			msg = ((uint32_t)MEVT_SHORTMSG << 24) | ((uint32_t)evt.data[bytesRead++] << 8) | ((uint32_t)_track.last_event.event);
			if ((_track.last_event.event & 0xf0) != 0xc0 && (_track.last_event.event & 0xf0) != 0xd0)
			{
				msg |= (evt.data[bytesRead++] << 16);
			}
			midiOutShortMsg(_midiOut, msg);
		}
		else if (evt.event == 0xff) // meta-event
		{
			bytesRead++; // skip the event byte
			meta = evt.data[bytesRead++]; // read the meta-event byte
			len = evt.data[bytesRead++];
			if (meta == 0x51)
			{
				// only care about tempo events
				_ppqnClock = (((uint32_t)evt.data[bytesRead++] << 16) | ((uint32_t)evt.data[bytesRead++] << 8) | ((uint32_t)evt.data[bytesRead++])) / _ticks;
			}
			else
			{
				// skip all other meta events
				bytesRead += len;
			}
		}
		else if ((evt.event & 0xf0) != 0xf0) // normal command
		{
			_track.last_event = evt;
			bytesRead++; // skip the event byte
			msg = ((uint32_t)MEVT_SHORTMSG << 24) | ((uint32_t)evt.data[bytesRead++] << 8) | ((uint32_t)evt.event);
			if ((evt.event & 0xf0) != 0xc0 && (evt.event & 0xf0) != 0xd0)
			{
				msg |= (evt.data[bytesRead++] << 16);
			}
			midiOutShortMsg(_midiOut, msg);
			_cmdMsgs.push_back(msg);
		}

		_track.buf = evt.data + bytesRead;

		_mutex.unlock();
	}
}

void MidiPlayer::reset()
{
	const uint8_t* ptr = _soundData.data() + sizeof(uint64_t) + sizeof(uint32_t);
	_ticks = (uint16_t)ptr[0] << 8 | ptr[1];
	_ppqnClock = 500000U / _ticks;
	_track.buf = ptr + 10;
	_track.absoluteTime = 0;
	_track.last_event = {};
	_currentTime = 0;
	_state = PlaySyncState::NeedPlay;
	_cmdMsgs.clear();
	_isPlaying = false;
}
MidiPlayer::MidiEvent MidiPlayer::getNextEvent()
{
	MidiEvent e = {};
	uint32_t bytesRead = 0, time = 0;
	uint8_t c;

	do {
		c = _track.buf[bytesRead++];
		time = (time << 7) + (c & 0x7f);
	} while (c & 0x80);

	e.data = _track.buf + bytesRead;
	e.absoluteTime = _track.absoluteTime + time;
	e.event = *e.data;
	return e;
}

void MidiPlayer::usleep(uint32_t waitTime)
{
	if (waitTime > 0)
	{
		LARGE_INTEGER startTime, endTime;
		QueryPerformanceCounter(&startTime);
		do QueryPerformanceCounter(&endTime);
		while ((endTime.QuadPart - startTime.QuadPart) * 1000000ll / PerformanceFrequency.QuadPart < waitTime);
	}
}
