#pragma once

#include "Framework/Framework.h"


struct MidiFile
{
	vector<uint8_t> data;

	MidiFile(const vector<uint8_t>& xmiData);
};
