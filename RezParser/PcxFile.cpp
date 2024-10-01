#include "PcxFile.h"


PcxFile::PcxFile(shared_ptr<BufferReader> pcxReader)
{
	// based on https://github.com/freudi74/mfimage (I copied only what I need)


	uint32_t maxXPadding, plane = 0, x = 0, y = 0;
	uint16_t startX, startY, endX, endY, bytesPerLine;
	uint8_t pcxVer, pcxId, pcxReserved1, numBitPlanes, runCount = 0, runValue, i, byteValue, u8;
	uint8_t pcxEnc; // encoding: 0 = raw; 1 = RLE
	uint8_t bpp; // bits per pixel (or: plane)
	int8_t bit = 0;


	pcxReader->read(pcxId);
	pcxReader->read(pcxVer);
	pcxReader->read(pcxEnc);
	pcxReader->read(bpp);
	pcxReader->read(startX);
	pcxReader->read(startY);
	pcxReader->read(endX);
	pcxReader->read(endY);
	pcxReader->skip(4);

	// 16 color palette
	for (i = 0; i < 16; i++)
	{
		pcxReader->read(palette.colors[i].r);
		pcxReader->read(palette.colors[i].g);
		pcxReader->read(palette.colors[i].b);
		palette.colors[i].a = 255;
	}

	pcxReader->read(pcxReserved1);
	pcxReader->read(numBitPlanes);
	pcxReader->read(bytesPerLine);
	pcxReader->skip(60);


	if (pcxId != 0x0a || pcxVer != 5 || pcxVer == 1 || pcxEnc > 1 || endX <= startX || endY <= startY || pcxReserved1 || numBitPlanes != 1)
	{
		throw Exception("not PCX or unsupported version");
	}

	width = endX - startX + 1;
	height = endY - startY + 1;
	colors = DynamicArray<ColorRGBA>((size_t)width * height);
	maxXPadding = bytesPerLine * 8 / bpp;


	if (bpp > 4)
	{
		// verify that the byte end-769 is 0xc0
		const int64_t endOfHeader = pcxReader->getIndex();
		pcxReader->setIndex(pcxReader->getSize() - 769);

		if (pcxReader->read<uint8_t>() == 0x0c)
		{
			// 256 color palette
			for (uint16_t i = 0; i < 256; i++)
			{
				pcxReader->read(palette.colors[i].r);
				pcxReader->read(palette.colors[i].g);
				pcxReader->read(palette.colors[i].b);
				palette.colors[i].a = 255;
			}
		}
		pcxReader->setIndex(endOfHeader);
	}

	palette.colors[0].a = 0; // first pixel in palette is transparent


	while (y < height)
	{
		u8 = pcxReader->read<uint8_t>();

		if (pcxEnc == 1) // is RLE
		{
			if ((u8 & 0xC0) == 0xC0) // 2-byte code
			{
				runCount = u8 & 0x3F;
				pcxReader->read(runValue);
			}
			else // 1-byte code
			{
				runCount = 1;
				runValue = u8; // pixel value
			}
		}
		else
		{
			pcxReader->read(runValue);
		}

		for (i = 0; i < runCount && y < height; i++)
		{
			do {
				byteValue = 0;
				switch (bpp)
				{
				case 8: byteValue = runValue; break;
				case 4: byteValue = ((runValue >> (4 - bit)) & 0xf); bit += 4; break;
				case 2: byteValue = ((runValue >> (6 - bit)) & 0x3); bit += 2; break;
				case 1: byteValue = ((runValue >> (7 - bit)) & 0x1); bit += 1; break;
				}

				if (bit == 8)
				{
					bit = 0;
				}

				if (x >= width)
				{
					if (x < maxXPadding)
					{
						x++; // this is a "padding" byte/bit, just ignore it!
						continue;
					}
					else
					{
						// next plane
						x = 0;
						if (++plane >= numBitPlanes)
						{
							// next line
							y++;
							plane = 0;
						}
					}
				}

				if (y == height)
				{
					break; // we are done!
				}

				colors[(size_t)width * y + x++] = palette.colors[byteValue];
			} while (bit != 0);
		}
	}
}
