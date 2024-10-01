#pragma once

#include "Framework/Framework.h"
#include "Framework/DynamicArray.hpp"


struct MidiFile
{
	DynamicArray<uint8_t> data;

	MidiFile(const DynamicArray<uint8_t>& xmiData);
};
