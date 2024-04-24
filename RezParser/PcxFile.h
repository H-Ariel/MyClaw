#pragma once

#include "PalFile.h"
#include "Framework/BufferReader.h"


struct PcxFile
{
	PcxFile(shared_ptr<BufferReader> pcxReader);

	vector<ColorRGBA> colors;
	WapPal palette;
	uint32_t width, height;
};
