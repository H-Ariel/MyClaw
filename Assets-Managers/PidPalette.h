#pragma once

#include "RezArchive.h"


struct ColorRGBA
{
	uint8_t r, g, b, a;
};


class PidPalette
{
public:
	static const uint16_t COLORS_IN_PALETTE = 256;
	static const uint16_t PALETTE_SIZE_BYTES = COLORS_IN_PALETTE * 3; // only RGB are stored in .PAL file

	const ColorRGBA colors[COLORS_IN_PALETTE];

	PidPalette(const vector<uint8_t>& palData);
	PidPalette(ColorRGBA palColors[COLORS_IN_PALETTE]);
};
