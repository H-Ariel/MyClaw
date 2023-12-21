#include "MidiPlayer.h"


#ifdef _DEBUG
// prints the MIDI error and returns it for further use
static void MIDI_CALL(MMRESULT mmResult)
{
	if (mmResult != MMSYSERR_NOERROR)
	{
		char text[256] = {};
		midiOutGetErrorTextA(mmResult, text, sizeof(text));
		DBG_PRINT("MIDI error: %s\n", text);
	}
}
#else
#define MIDI_CALL(func) func
#endif

static const LARGE_INTEGER EMPTY_LARGE_INTEGER = {};

float MidiPlayer::MusicSpeed = 1;
HMIDIOUT MidiPlayer::_midiOut = nullptr;
int MidiPlayer::_objCount = 0;
LARGE_INTEGER MidiPlayer::PerformanceFrequency = {};


vector<uint8_t> xmiToMidi(vector<uint8_t> xmiFileData); // impleted in "XmiFile.cpp"


MidiPlayer::MidiPlayer(const vector<uint8_t>& xmiFileData)
	: _midiData(xmiToMidi(xmiFileData)), track({}),
	PPQN_CLOCK(0), ticks(0), _isPlaying(false)
{
	if (!memcmp(&PerformanceFrequency, &EMPTY_LARGE_INTEGER, sizeof(LARGE_INTEGER)))
		QueryPerformanceFrequency(&PerformanceFrequency);

	if (_objCount++ == 0)
	{
		// open the MIDI device if it's the first instance
		MIDI_CALL(midiOutOpen(&_midiOut, 0, 0, 0, CALLBACK_NULL));
	}
}

MidiPlayer::~MidiPlayer()
{
	stop();
	if (--_objCount == 0)
	{
		// close the MIDI device if it's the last instance
		MIDI_CALL(midiOutClose(_midiOut));
		_midiOut = nullptr;
	}
}

void MidiPlayer::stop()
{
	_mutex.lock();
	usleep(100000); // wait 1 second for the last note to finish playing (so mutex won't crash)
	_isPlaying = false;
	midiOutReset(_midiOut); // turn off all notes and reset the MIDI device
	_mutex.unlock();
}

void MidiPlayer::play(bool infinite)
{
	reset();
	_isPlaying = true;
	thread(&MidiPlayer::play_sync, this, infinite).detach();
}

void MidiPlayer::reset()
{
	stop();

	uint8_t* buf = _midiData.data();
	MidiHeader hdr = *((MidiHeader*)buf);
	ticks = reverseBytes(hdr.ticks);
	PPQN_CLOCK = 500000 / ticks;
	buf += sizeof(MidiHeader);
	track.track = (MidiTrackData*)buf;
	track.buf = buf + sizeof(MidiTrackData);
	track.absolute_time = 0;
	track.last_event;
	buf += sizeof(MidiTrackData) + reverseBytes(track.track->length);
}

void MidiPlayer::play_sync(bool infinite)
{
	MidiEvent evt;
	uint32_t current_time, time, bytesread, msg, len;
	uint8_t c;

start:
	current_time = 0;

	while (_isPlaying)
	{
		time = -1;
		bytesread = 0;
		evt = getNextEvent();

		if ((evt.event == 0xFF && evt.data[1] == 0x2F) || evt.absolute_time > time) // finish playing music
		{
			if (infinite)
			{
				reset();
				_isPlaying = true;
				goto start;
			}
			else 
			{
				stop();
			}
		}
		else
			time = evt.absolute_time;

		if (!_isPlaying)
		{
			break;
		}

		_mutex.lock();

		evt = getNextEvent();

		track.absolute_time = evt.absolute_time;
		time = track.absolute_time - current_time;
		current_time = track.absolute_time;

		_mutex.unlock();
		usleep(time * PPQN_CLOCK);
		if (!_isPlaying)
		{
			break;
		}
		_mutex.lock();

		if (!(evt.event & 0x80)) // running mode
		{
			MidiEvent last = track.last_event;
			c = evt.data[bytesread++]; // get the first data byte
			msg = ((uint32_t)MEVT_SHORTMSG << 24) | ((uint32_t)c << 8) | ((uint32_t)last.event);

			if ((last.event & 0xF0) != 0xC0 && (last.event & 0xF0) != 0xD0)
			{
				c = evt.data[bytesread++]; // get the second data byte
				msg |= ((uint32_t)c << 16);
			}

			if (_isPlaying)
				MIDI_CALL(midiOutShortMsg(_midiOut, msg));

			track.buf = evt.data + bytesread;
		}
		else if (evt.event == 0xFF) // meta-event
		{
			bytesread++; // skip the event byte

			if (evt.data[bytesread++] == 0x51) // read the meta-event byte. only care about tempo events
			{
				uint8_t a, b, c;
				len = evt.data[bytesread++]; // get the length byte, should be 3
				a = evt.data[bytesread++];
				b = evt.data[bytesread++];
				c = evt.data[bytesread++];
				msg = ((uint32_t)a << 16) | ((uint32_t)b << 8) | ((uint32_t)c);
				PPQN_CLOCK = msg / ticks;
			}
			else // skip all other meta events
			{
				bytesread += evt.data[bytesread++]; // pass acorrding to the length byte
			}

			track.buf = evt.data + bytesread;
		}
		else if ((evt.event & 0xF0) != 0xF0) // normal command
		{
			track.last_event = evt;
			bytesread++; // skip the event byte
			c = evt.data[bytesread++]; // get the first data byte
			msg = ((uint32_t)MEVT_SHORTMSG << 24) | ((uint32_t)c << 8) | ((uint32_t)evt.event);

			if ((evt.event & 0xF0) != 0xC0 && (evt.event & 0xF0) != 0xD0)
			{
				c = evt.data[bytesread++]; // get the second data byte
				msg |= ((uint32_t)c << 16);
			}

			if (_isPlaying)
				MIDI_CALL(midiOutShortMsg(_midiOut, msg));

			track.buf = evt.data + bytesread;
		}

		_mutex.unlock();
	}
}

MidiPlayer::MidiEvent MidiPlayer::getNextEvent() const
{
	uint32_t bytesread = 0, time = 0;
	uint8_t* buf = track.buf;
	uint8_t c;

	do {
		c = buf[bytesread++];
		time = (time << 7) + (c & 0x7f);
	} while (c & 0x80);

	buf += bytesread;

	MidiEvent evt = {};
	evt.absolute_time = track.absolute_time + time;
	evt.data = buf;
	evt.event = *buf;
	return evt;
}

void MidiPlayer::usleep(int waitTime)
{
	if (waitTime != 0)
	{
		waitTime = int(waitTime / MusicSpeed);
		LARGE_INTEGER time1, time2;
		QueryPerformanceCounter(&time1);
		do QueryPerformanceCounter(&time2); 
		while ((time2.QuadPart - time1.QuadPart) * 1000000ll / PerformanceFrequency.QuadPart < waitTime);
	}
}
