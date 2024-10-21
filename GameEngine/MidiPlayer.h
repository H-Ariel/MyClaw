// midi player based on https://blog.fourthwoods.com/2012/02/24/playing-midi-files-in-windows-part-5/

#pragma once

#include "Framework/Framework.h"
#include "Framework/DynamicArray.hpp"


// MIDI player class
class MidiPlayer
{
public:
	MidiPlayer(const string& key, const DynamicArray<uint8_t>& midiData);
	~MidiPlayer();

	void play(bool infinite); // play the MIDI file asynchronously
	void stop(); // stop the MIDI player
	void reset(); // reset the track of current midi-player
	static void setVolume(float volume); // set the volume. `volume` range is [0,1]

	bool isPlaying() const { return _isPlaying; }
	const string& getKey() const { return _key; }

	static float MusicSpeed;

private:
#pragma pack(push, 1)

	struct MidiHeader
	{
		uint32_t id; // identifier "MThd"
		uint32_t size; // always 6 in big-endian format
		uint16_t format; // big-endian format
		uint16_t tracks; // number of tracks, big-endian
		uint16_t ticks; // number of ticks per quarter note, big-endian
	};

	struct MidiTrackData
	{
		uint32_t id; // identifier "MTrk"
		uint32_t length; // track length, big-endian
	};

#pragma pack(pop)

	struct MidiEvent
	{
		uint32_t absolute_time;
		uint8_t* data;
		uint8_t event;
	};

	struct MidiTrack
	{
		MidiTrackData* track;
		uint8_t* buf;
		MidiEvent last_event;
		uint32_t absolute_time;
	};

	// play the MIDI file synchronously (used only in thread)
	void play_sync(bool infinite);

	// get the next event from the track
	MidiEvent getNextEvent() const;

	// sleep for waitTime microseconds (better than Sleep() which sleeps for milliseconds)
	static void usleep(int waitTime);


	const string _key;
	DynamicArray<uint8_t> _soundData;
	bool _isPlaying;

	mutex _mutex;
	thread* _thread; // thread for playing the MIDI asynchronously
	MidiTrack track; // in our MIDI data we know there's only one track
	uint32_t PPQN_CLOCK;
	uint16_t ticks; // from the header

	static HMIDIOUT _midiOut; // MIDI device handle (static because it's shared between instances)
	static int _objCount; // number of instances of this class
	static LARGE_INTEGER PerformanceFrequency;
};
