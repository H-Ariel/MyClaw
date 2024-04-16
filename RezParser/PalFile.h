#pragma once

#include "RezArchiveFile.h"


struct ColorRGBA
{
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
};

class WapPal
{
public:
	static const uint16_t WAP_COLORS_IN_PALETTE = 256;
	static const uint16_t WAP_PALETTE_SIZE_BYTES = WAP_COLORS_IN_PALETTE * 3; // only RGB are stored, not alpha

	WapPal();
	WapPal(const vector<uint8_t>& data);
	WapPal(const ColorRGBA palette[WAP_COLORS_IN_PALETTE]);

	ColorRGBA colors[WAP_COLORS_IN_PALETTE];
};
