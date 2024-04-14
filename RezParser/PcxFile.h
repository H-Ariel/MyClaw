#pragma once

#include "PalFile.h"


struct PcxFile
{
	PcxFile(shared_ptr<BufferReader> pcxReader);
	PcxFile(const PcxFile& other);
	~PcxFile();

	vector<ColorRGBA> colors;
	WapPal* palette;
	uint32_t width, height;
};
