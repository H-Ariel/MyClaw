// midi player based on https://blog.fourthwoods.com/2012/02/24/playing-midi-files-in-windows-part-5/

#pragma once

#include "framework.h"


class MidiPlayer
{
public:
	MidiPlayer(vector<uint8_t> xmiFileData);
	~MidiPlayer();

	void play(bool infinite); // start sound the MIDI file (or continue)
	void stop(); // stop (or puase)
	void reset();
	bool isPlaying() const { return _isPlaying; }

private:
	struct MidiEvent
	{
		const uint8_t* data;
		uint32_t absoluteTime;
		uint8_t event;
	};

	struct MidiTrack
	{
		MidiEvent last_event;
		const uint8_t* buf;
		uint32_t absoluteTime;
	};


	enum class PlaySyncState : int8_t { NeedPlay = 0, EndPlay = -1 };


	void playSync(bool infinite);
	MidiEvent getNextEvent();
	static void usleep(uint32_t waitTime);


	mutex _mutex;
	const vector<uint8_t> _soundData;
	vector<uint32_t> _cmdMsgs; // save the command messages
	MidiTrack _track;
	HMIDIOUT _midiOut;
	uint32_t _ppqnClock;
	uint32_t _currentTime;
	uint16_t _ticks; // number of ticks per quarter note
	PlaySyncState _state;
	bool _isPlaying;
};
