#include "MidiPlayer.h"


//#define MIDI_NO_PRINT // comment this line to print MIDI events
#define MIDI_CALL(x) printMidiError(x, __LINE__)

#ifdef MIDI_NO_PRINT
static const char* notes[] = {
	"C - 0", "C# - 0", "D - 0", "D# - 0", "E - 0", "F - 0", "F# - 0", "G - 0", "G# - 0", "A - 0", "A# - 0", "B - 0",
	"C - 1", "C# - 1", "D - 1", "D# - 1", "E - 1", "F - 1", "F# - 1", "G - 1", "G# - 1", "A - 1", "A# - 1", "B - 1",
	"C - 2", "C# - 2", "D - 2", "D# - 2", "E - 2", "F - 2", "F# - 2", "G - 2", "G# - 2", "A - 2", "A# - 2", "B - 2",
	"C - 3", "C# - 3", "D - 3", "D# - 3", "E - 3", "F - 3", "F# - 3", "G - 3", "G# - 3", "A - 3", "A# - 3", "B - 3",
	"C - 4", "C# - 4", "D - 4", "D# - 4", "E - 4", "F - 4", "F# - 4", "G - 4", "G# - 4", "A - 4", "A# - 4", "B - 4",
	"C - 5", "C# - 5", "D - 5", "D# - 5", "E - 5", "F - 5", "F# - 5", "G - 5", "G# - 5", "A - 5", "A# - 5", "B - 5",
	"C - 6", "C# - 6", "D - 6", "D# - 6", "E - 6", "F - 6", "F# - 6", "G - 6", "G# - 6", "A - 6", "A# - 6", "B - 6",
	"C - 7", "C# - 7", "D - 7", "D# - 7", "E - 7", "F - 7", "F# - 7", "G - 7", "G# - 7", "A - 7", "A# - 7", "B - 7",
	"C - 8", "C# - 8", "D - 8", "D# - 8", "E - 8", "F - 8", "F# - 8", "G - 8", "G# - 8", "A - 8", "A# - 8", "B - 8",
	"C - 9", "C# - 9", "D - 9", "D# - 9", "E - 9", "F - 9", "F# - 9", "G - 9", "G# - 9", "A - 9", "A# - 9", "B - 9",
	"C - 10", "C# - 10", "D - 10", "D# - 10", "E - 10", "F - 10", "F# - 10", "G - 10"
};

static const char* patch[] = {
	"Acoustic Grand", "Bright Acoustic", "Electric Grand", "Honky-Tonk", "Electric Piano 1",
	"Electric Piano 2", "Harpsichord", "Clav", "Celesta", "Glockenspiel", "Music Box",
	"Vibraphone", "Marimba", "Xylophone", "Tubular Bells", "Dulcimer", "Drawbar Organ",
	"Percussive Organ", "Rock Organ", "Church Organ", "Reed Organ", "Accoridan", "Harmonica",
	"Tango Accordian", "Acoustic Guitar(nylon)", "Acoustic Guitar(steel)",
	"Electric Guitar(jazz)", "Electric Guitar(clean)", "Electric Guitar(muted)",
	"Overdriven Guitar", "Distortion Guitar", "Guitar Harmonics", "Acoustic Bass",
	"Electric Bass(finger)", "Electric Bass(pick)", "Fretless Bass", "Slap Bass 1",
	"Slap Bass 2", "Synth Bass 1", "Synth Bass 2", "Violin", "Viola", "Cello", "Contrabass",
	"Tremolo Strings", "Pizzicato Strings", "Orchestral Strings", "Timpani", "String Ensemble 1",
	"String Ensemble 2", "SynthStrings 1", "SynthStrings 2", "Choir Aahs", "Voice Oohs",
	"Synth Voice", "Orchestra Hit", "Trumpet", "Trombone", "Tuba", "Muted Trumpet",
	"French Horn", "Brass Section", "SynthBrass 1", "SynthBrass 2", "Soprano Sax", "Alto Sax",
	"Tenor Sax", "Baritone Sax", "Oboe", "English Horn", "Bassoon", "Clarinet", "Piccolo",
	"Flute", "Recorder", "Pan Flute", "Blown Bottle", "Skakuhachi", "Whistle", "Ocarina",
	"Lead 1 (square)", "Lead 2 (sawtooth)", "Lead 3 (calliope)", "Lead 4 (chiff)",
	"Lead 5 (charang)", "Lead 6 (voice)", "Lead 7 (fifths)", "Lead 8 (bass+lead)", "Pad 1 (new age)",
	"Pad 2 (warm)", "Pad 3 (polysynth)", "Pad 4 (choir)", "Pad 5 (bowed)", "Pad 6 (metallic)",
	"Pad 7 (halo)", "Pad 8 (sweep)", "FX 1 (rain)", "FX 2 (soundtrack)", "FX 3 (crystal)",
	"FX 4 (atmosphere)", "FX 5 (brightness)", "FX 6 (goblins)", "FX 7 (echoes)", "FX 8 (sci-fi)",
	"Sitar", "Banjo", "Shamisen", "Koto", "Kalimba", "Bagpipe", "Fiddle", "Shanai", "Tinkle Bell",
	"Agogo", "Steel Drums", "Woodblock", "Taiko Drum", "Melodic Tom", "Synth Drum", "Reverse Cymbal",
	"Guitar Fret Noise", "Breath Noise", "Seashore", "Bird Tweet", "Telephone Ring", "Helicopter",
	"Applause", "Gunshot"
};

static const char* percussion[] = {
	"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
	"Acoustic Bass Drum", "Bass Drum 1", "Side Stick", "Acoustic Snare", "Hand Clap",
	"Electric Snare", "Low Floor Tom", "Closed Hi-Hat", "High Floor Tom", "Pedal Hi-Hat",
	"Low Tom", "Open Hi-Hat", "Low-Mid Tom", "Hi-Mid Tom", "Crash Cymbal 1",
	"High Tom", "Ride Cymbal 1", "Chinese Cymbal", "Ride Bell", "Tambourine",
	"Splash Cymbal", "Cowbell", "Crash Cymbal 2", "Vibraslap", "Ride Cymbal 2",
	"Hi Bongo", "Low Bongo", "Mute Hi Conga", "Open Hi Conga", "Low Conga",
	"High Timbale", "Low Timbale", "High Agogo", "Low Agogo", "Cabasa",
	"Maracas", "Short Whistle", "Long Whistle", "Short Guiro", "Long Guiro",
	"Claves", "Hi Wood Block", "Low Wood Block", "Mute Cuica", "Open Cuica",
	"Mute Triangle", "Open Triangle"
};

void MidiPlayer::MidiEvent::print() const
{
	uint8_t event = this->event & 0xF0;
	uint8_t channel = this->event & 0x0F;
	uint8_t data1 = this->data[1], data2 = 0;

	if (event != 0xc0 && event != 0xd0)
		data2 = this->data[2]; // get the second data byte

	switch (event)
	{
	case 0x80:
		printf("Note off, channel %d, ", channel);
		printf("%s, %d\n", (channel != 9) ? notes[data1] : percussion[data1], data2);
		break;
	case 0x90:
		printf("Note on, channel %d, ", channel);
		printf("%s, %d\n", (channel != 9) ? notes[data1] : percussion[data1], data2);
		break;
	case 0xA0:
		printf("Key pressure, channel %d, ", channel);
		printf("%s, %d\n", (channel != 9) ? notes[data1] : percussion[data1], data2);
		break;
	case 0xB0:
		printf("Controller change, channel %d, ", channel);
		printf("%d, %d\n", data1, data2);
		break;
	case 0xC0:
		printf("Patch change, channel %d, ", channel);
		printf("%s\n", patch[data1]);
		break;
	case 0xD0:
		printf("Channel pressure, channel %d, ", channel);
		printf("%d\n", data1);
		break;
	case 0xE0:
		printf("Pitch wheel, channel %d, ", channel);
		printf("%04x\n", (data1 & 0x7F) | ((uint32_t)(data2 & 0x7F)) << 7);
		break;
	case 0xF0:
		printf("Meta-event\n");
		break;
	}
}
#else 
void MidiPlayer::MidiEvent::print() const {}
#endif

static const LARGE_INTEGER EMPTY_LARGE_INTEGER = {};

HMIDIOUT MidiPlayer::_midiOut = nullptr;
int MidiPlayer::_objCount = 0;
LARGE_INTEGER MidiPlayer::PerformanceFrequency = {};


vector<uint8_t> xmiToMidi(vector<uint8_t> xmiFileData); // impleted in "XmiFile.cpp"

// prints the MIDI error and returns it for further use
MMRESULT printMidiError(MMRESULT err, int line)
{
	if (err != MMSYSERR_NOERROR)
	{
		char txt[256] = {};
		midiOutGetErrorTextA(err, txt, 256);
		printf("MIDI error at line %d: %s\n", line, txt);
	}
	return err;
}

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

#ifdef MIDI_NO_PRINT
	printf("Track, length: %d\n", reverseBytes(track.track->length));
#endif

start:
	current_time = 0;

	while (_isPlaying)
	{
		time = -1;
		bytesread = 0;
		evt = getNextEvent();

		if ((evt.event == 0xFF && evt.data[1] == 0x2F) || evt.absolute_time > time)
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

#ifdef MIDI_NO_PRINT
		printf("Track: ticks: %d: ", track.absolute_time);
#endif

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

#ifdef MIDI_NO_PRINT
			last.data = DBG_NEW uint8_t[4];
			last.data[0] = last.event;
			last.data[1] = evt.data[0];
			last.data[2] = evt.data[1];
			printf("+ ");
			last.print();
			delete[] last.data;
#endif

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
#ifdef MIDI_NO_PRINT
			evt.print();
#endif

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
#ifdef MIDI_NO_PRINT
			evt.print();
#endif				

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

	MidiEvent evt;
	evt.absolute_time = track.absolute_time + time;
	evt.data = buf;
	evt.event = *buf;
	return evt;
}

void MidiPlayer::usleep(int waitTime)
{
	if (waitTime != 0)
	{
		LARGE_INTEGER time1, time2;

		QueryPerformanceCounter(&time1);

		do {
			QueryPerformanceCounter(&time2);
		} while ((time2.QuadPart - time1.QuadPart) * 1000000ll / PerformanceFrequency.QuadPart < waitTime);
	}
}
