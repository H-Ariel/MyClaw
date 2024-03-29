#include "PidPalette.h"


PidPalette::PidPalette(const vector<uint8_t>& palData)
	: colors()
{
	if (palData.size() == PALETTE_SIZE_BYTES)
	{
		for (uint16_t i = 0, j = 0; i < COLORS_IN_PALETTE; i++)
		{
			myMemCpy(colors[i].r, palData[j++]);
			myMemCpy(colors[i].g, palData[j++]);
			myMemCpy(colors[i].b, palData[j++]);
			myMemCpy<uint8_t>(colors[i].a, 0xFF);
		}
		myMemCpy<uint8_t>(colors[0].a, 0); // first pixel in palette is transparent
	}
}

PidPalette::PidPalette(ColorRGBA palColors[COLORS_IN_PALETTE])
	: colors()
{
	memcpy((ColorRGBA*)colors, palColors, sizeof(colors));
}
