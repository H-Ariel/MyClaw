#include "PalFile.h"


WapPal::WapPal(const vector<uint8_t>& data)
{
	if (data.size() != WAP_PALETTE_SIZE_BYTES)
	{
		throw Exception("Invalid PAL file");
	}

	// Read whole palette, there is no header
	for (uint16_t i = 0, j = 0; i < WAP_COLORS_IN_PALETTE; i++)
	{
		colors[i].r = data[j++];
		colors[i].g = data[j++];
		colors[i].b = data[j++];
		colors[i].a = 0xFF;
	}

	colors[0].a = 0; // First pixel in palette is transparent
}
WapPal::WapPal(const ColorRGBA palette[WAP_COLORS_IN_PALETTE])
{
	memcpy(colors, palette, sizeof(colors));
}
