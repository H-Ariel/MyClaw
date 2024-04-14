#include "PidFile.h"


WapPid::WapPid(const vector<uint8_t>& data, WapPal* palette)
{
	uint32_t x, y, i;
	size_t size = data.size();
	uint8_t byte;

	/********************** PID HEADER/PROPERTIES **********************/

	BufferReader pidFileStream(data);

	pidFileStream.skip(4); // fileDesc
	pidFileStream.read(flags);
	pidFileStream.read(width);
	pidFileStream.read(height);
	pidFileStream.read(offsetX);
	pidFileStream.read(offsetY);
	pidFileStream.skip(8); // skip unknown data

	/********************** PID PALETTE **********************/

	WapPal* imagePalette = nullptr;

	// If image has embedded palette within it, extract it
	if (flags & PidFlag_EmbeddedPalette)
	{
		uint32_t paletteOffset = uint32_t(size - WapPal::WAP_PALETTE_SIZE_BYTES);
		const uint8_t* paletteData = &(data[paletteOffset]);
		imagePalette = DBG_NEW WapPal(vector<uint8_t>(paletteData, paletteData + WapPal::WAP_PALETTE_SIZE_BYTES));
	}
	else
	{
		imagePalette = palette;
	}

	// Make sure we have loaded a palette
	if (imagePalette == nullptr)
	{
		throw Exception("PID file has no palette");
	}

	/********************** PID PIXELS **********************/

	colors.resize(width * height);

	x = 0;
	y = 0;
	// PID is compressed, RLE
	if (flags & PidFlag_Compression)
	{
		while (y < height)
		{
			pidFileStream.read(byte);

			if (byte > 128)
			{
				for (i = byte - 128; i > 0 && y < height; i--)
				{
					//colors[y * width + x] = ColorRGBA{ 0, 0, 0, 1 };
					x++;
					if (x == width)
					{
						x = 0;
						y++;
					}
				}
			}
			else
			{
				for (i = byte; i > 0 && y < height; i--)
				{
					pidFileStream.read(byte);

					colors[y * width + x] = imagePalette->colors[byte];

					x++;
					if (x == width)
					{
						x = 0;
						y++;
					}
				}
			}
		}
	}
	else
	{
		while (y < height)
		{
			i = 1;
			pidFileStream.read(byte);

			// PID related encoding probably, this means how many same pixels are following.
			// e.g. if byte = 220, then 220-192=28 same pixels are next to each other
			if (byte > 192)
			{
				i = byte - 192;
				pidFileStream.read(byte);
			}

			for (; i > 0 && y < height; i--)
			{
				colors[y * width + x] = imagePalette->colors[byte];

				x++;
				if (x == width)
				{
					x = 0;
					y++;
				}
			}
		}
	}

	// If we created new palette, destroy it
	if (flags & PidFlag_EmbeddedPalette)
		delete imagePalette;
}
