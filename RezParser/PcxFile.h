#pragma once

#include "PalFile.h"
#include "Framework/BufferReader.h"


struct PcxFile
{
	PcxFile(shared_ptr<BufferReader> pcxReader);

	DynamicArray<ColorRGBA> colors;
	WapPal palette; // store pallette from PCX file
	uint32_t width, height;
};
