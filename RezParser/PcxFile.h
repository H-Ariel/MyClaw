#pragma once

#include "PalFile.h"
#include "Framework/Buffer.h"


struct PcxFile
{
	PcxFile(shared_ptr<Buffer> pcxReader);

	DynamicArray<ColorRGBA> colors;
	WapPal palette; // store pallette from PCX file
	uint32_t width, height;
};
