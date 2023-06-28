// midi player based on https://blog.fourthwoods.com/2012/02/24/playing-midi-files-in-windows-part-5/

#pragma once

#include "framework.h"

//#define MIDI_NO_PRINT // comment this line to print MIDI events

// MIDI player class
class MidiPlayer
{
public:
	MidiPlayer(const vector<uint8_t>& xmiFileData);
	~MidiPlayer();

	bool isPlaying() const { return _isPlaying; }

	// stop the MIDI player
	void stop();

	// play the MIDI file asynchronously
	void play(bool infinite);

	// reset the track of current midi-player
	void reset();
	
private:
#pragma pack(push, 1)

	struct MidiHeader {
		uint32_t id; // identifier "MThd"
		uint32_t size; // always 6 in big-endian format
		uint16_t format; // big-endian format
		uint16_t tracks; // number of tracks, big-endian
		uint16_t ticks; // number of ticks per quarter note, big-endian
	};

	struct MidiTrackData {
		uint32_t id; // identifier "MTrk"
		uint32_t length; // track length, big-endian
	};

#pragma pack(pop)

	struct MidiEvent {
		uint32_t absolute_time;
		uint8_t* data;
		uint8_t event;
	};

	struct MidiTrack {
		MidiTrackData* track;
		uint8_t* buf;
		MidiEvent last_event;
		uint32_t absolute_time;
	};

	// play the MIDI file synchronously
	void play_sync(bool infinite);

	// get the next event from the track
	MidiEvent get_next_event() const;

	// check if the event is the end of the track
	static bool is_track_end(const MidiEvent& e);

#ifdef MIDI_NO_PRINT
	// print the event to the console
	static void print_event(const MidiEvent& evt);
#endif

private:
	vector<uint8_t> _midiData;
	MidiTrack track; // in our MIDI data we know there's only one track
	uint32_t PPQN_CLOCK;
	uint16_t ticks; // from the header
	bool _isPlaying;

	static HMIDIOUT _midiOut; // MIDI device handle (static because it's shared between instances)
	static int _objCount; // number of instances of this class

};
