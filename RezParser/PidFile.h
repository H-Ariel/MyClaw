#pragma once

#include "PalFile.h"


enum PidFlags
{
	PidFlag_Transparency = 1 << 0,
	PidFlag_VideoMemory = 1 << 1,
	PidFlag_SystemMemory = 1 << 2,
	PidFlag_Mirror = 1 << 3,
	PidFlag_Invert = 1 << 4,
	PidFlag_Compression = 1 << 5,
	PidFlag_Lights = 1 << 6,
	PidFlag_EmbeddedPalette = 1 << 7,
};

struct WapPid
{
	WapPid(const vector<uint8_t>& data, WapPal* palette);


	vector<ColorRGBA> colors;
	uint32_t flags;
	uint32_t width, height;
	int32_t offsetX, offsetY;
};
