#pragma once

#include "RezArchive.h"


struct ColorRGBA
{
	uint8_t r, g, b, a;
};


class PidPalette
{
public:
	PidPalette(vector<uint8_t> palData);

	
	static const uint16_t COLORS_IN_PALETTE = 256;
	static const uint16_t PALETTE_SIZE_BYTES = COLORS_IN_PALETTE * 3; // only RGB are stored in .PAL file

	const ColorRGBA colors[COLORS_IN_PALETTE];
};
