#pragma once

#include "RezArchiveFile.h"


/***************************************************************/
/********************* XMI FORMAT ******************************/
/***************************************************************/

struct MidiFile
{
	vector<uint8_t> data;

	MidiFile(const vector<uint8_t>& xmiData);
};
