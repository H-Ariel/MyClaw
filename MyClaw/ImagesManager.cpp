#include "ImagesManager.h"
#include "WindowManager.h"


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


ImagesManager::ImagesManager(RezArchive* rezArchive) : _rezArchive(rezArchive) {}
ImagesManager::~ImagesManager()
{
	for (auto& i : _loadedBitmaps)
	{
		SafeRelease(i.second.first);
	}
}

shared_ptr<UIBaseImage> ImagesManager::loadImage(const string& path)
{
	shared_ptr<UIBaseImage> img;

	if (_loadedBitmaps.count(path))
	{
		img = allocNewSharedPtr<UIBaseImage>(_loadedBitmaps[path].first, _loadedBitmaps[path].second);
	}
	else
	{
		try
		{
			if (endsWith(path, ".PID"))
			{
				img = loadPidImage(path);
			}
			else if (endsWith(path, ".PCX"))
			{
				img = loadPcxImage(path);
			}
		}
		catch (const Exception& ex)
		{
			img = allocNewSharedPtr<UIBaseImage>(nullptr); // empty image
			//cout << "WARNING: error while loading image \"" << path << "\". message: " << ex.what() << endl;
			cout << "WARNING: a blank image has been inserted. image path: \"" << path << '"' << endl;
		}

		_loadedBitmaps[path] = { img->_bitmap, img->offset };
	}

	return img;
}
void ImagesManager::clearLevelImages(const string& prefix)
{
	vector<string> keysToRemove;
	for (auto& a : _loadedBitmaps)
	{
		if (startsWith(a.first, prefix))
		{
			keysToRemove.push_back(a.first);
		}
	}

	for (const string& k : keysToRemove)
	{
		_loadedBitmaps.erase(k);
	}

	// this palette is from "STATES/BOOTY/SCREENS/001MAP.PCX" (all PCX of map has the same palette)
	static shared_ptr<PidPalette> menuPalette(DBG_NEW PidPalette(
		vector<uint8_t>({
			0x00, 0x00, 0x00, 0xff, 0xfc, 0xca, 0xf0, 0xee, 0x5a,
			0xdf, 0xdc, 0x4c, 0xdb, 0xd9, 0x4a, 0xd2, 0xd0, 0x48,
			0xbd, 0xbb, 0x3f, 0x9a, 0x99, 0x34, 0x82, 0x73, 0x2c,
			0x62, 0x61, 0x21, 0x40, 0x3f, 0x15, 0x29, 0x29, 0x0e,
			0x26, 0x23, 0x2c, 0x0d, 0x0b, 0x71, 0x13, 0x13, 0xaa,
			0x00, 0x68, 0x90, 0xb2, 0xbb, 0xbc, 0x12, 0x9d, 0x12,
			0x4b, 0x4a, 0x44, 0x70, 0x6f, 0x69, 0xdc, 0x60, 0x00,
			0x63, 0x04, 0x00, 0x8a, 0x8a, 0x8a, 0x7c, 0x7c, 0x7c,
			0x2c, 0x2c, 0x2c, 0x00, 0x00, 0x00, 0xfd, 0xf7, 0xca,
			0xfe, 0xf4, 0x91, 0xfc, 0xec, 0x43, 0xf7, 0xe8, 0x0b,
			0xf0, 0xd4, 0x5f, 0xe0, 0xb7, 0x1f, 0xcd, 0x9c, 0x0f,
			0xbe, 0x86, 0x12, 0xfa, 0xee, 0x8c, 0xef, 0xde, 0x61,
			0xd9, 0xc9, 0x5a, 0xc2, 0xb3, 0x4e, 0xa1, 0x95, 0x41,
			0x81, 0x77, 0x34, 0x5b, 0x00, 0x12, 0xfb, 0xdd, 0x1d,
			0xf7, 0xe3, 0x79, 0xdb, 0x88, 0x0f, 0xab, 0x49, 0x09,
			0x7d, 0x26, 0x11, 0xdb, 0xd9, 0xfe, 0x8f, 0x89, 0xfb,
			0x2d, 0x28, 0xda, 0x07, 0x06, 0xb6, 0xac, 0xfb, 0xcb,
			0x15, 0xe9, 0x3c, 0x00, 0x9e, 0x00, 0x00, 0x30, 0x00,
			0xf3, 0xc7, 0xfe, 0xca, 0x51, 0xea, 0x8f, 0x11, 0xb8,
			0x94, 0x00, 0xc2, 0xfe, 0xc1, 0xc2, 0xfd, 0x38, 0x39,
			0xf7, 0x02, 0x03, 0x92, 0x0b, 0x18, 0xfa, 0xde, 0x6d,
			0xfb, 0xef, 0xbf, 0xf2, 0xc6, 0x51, 0xf3, 0xbb, 0x30,
			0xcd, 0x8a, 0x0f, 0xe8, 0x98, 0x0d, 0x9d, 0x65, 0x18,
			0xdd, 0x96, 0x35, 0xc7, 0x8a, 0x39, 0xc4, 0x70, 0x19,
			0x8f, 0x43, 0x0a, 0x4f, 0x24, 0x05, 0x00, 0xff, 0x9c,
			0x08, 0x30, 0x79, 0xff, 0x00, 0xff, 0x5a, 0x0a, 0x3b,
			0xfa, 0xea, 0x98, 0xf1, 0xbc, 0x00, 0xee, 0xbd, 0x25,
			0xd1, 0x75, 0x05, 0x91, 0x48, 0x06, 0x0b, 0xae, 0x0b,
			0x7f, 0xd3, 0x81, 0x70, 0x32, 0x03, 0x09, 0x56, 0x00,
			0x46, 0xd0, 0x2e, 0xde, 0x12, 0xff, 0xe8, 0xfb, 0xd4,
			0x56, 0xdb, 0xd4, 0x00, 0x8e, 0xcb, 0x22, 0x60, 0xfa,
			0x12, 0x64, 0x1c, 0x37, 0x9d, 0x38, 0x07, 0x34, 0x03,
			0x78, 0xd0, 0x6e, 0xc0, 0x6d, 0x86, 0xc5, 0x2d, 0x5d,
			0x91, 0x12, 0x3a, 0x5f, 0x40, 0x1c, 0x6e, 0x4a, 0x20,
			0x12, 0x08, 0x2b, 0x29, 0x00, 0x00, 0x6a, 0x39, 0x08,
			0x00, 0x00, 0x00, 0x73, 0x63, 0x31, 0x73, 0x5a, 0x31,
			0x73, 0x5a, 0x29, 0x73, 0x52, 0x21, 0x73, 0x4b, 0x18,
			0x72, 0x4b, 0x10, 0x70, 0x42, 0x0e, 0x6d, 0x39, 0x10,
			0x6c, 0x39, 0x07, 0x63, 0x39, 0x07, 0x6c, 0x31, 0x09,
			0x63, 0x31, 0x10, 0x63, 0x31, 0x08, 0x64, 0x31, 0x00,
			0x5a, 0x31, 0x05, 0x64, 0x29, 0x05, 0x5a, 0x29, 0x08,
			0x5a, 0x29, 0x00, 0x51, 0x2a, 0x08, 0x52, 0x29, 0x00,
			0x5b, 0x20, 0x02, 0x50, 0x21, 0x08, 0x52, 0x21, 0x00,
			0x4a, 0x21, 0x00, 0x41, 0x22, 0x03, 0x52, 0x18, 0x00,
			0x4f, 0x18, 0x08, 0x4a, 0x18, 0x00, 0x41, 0x18, 0x00,
			0x4b, 0x10, 0x00, 0x3d, 0x10, 0x00, 0x2b, 0x0f, 0x00,
			0x2a, 0x2a, 0x2a, 0x23, 0x23, 0x23, 0x1d, 0x1d, 0x1d,
			0x19, 0x19, 0x19, 0x17, 0x17, 0x17, 0x15, 0x15, 0x15,
			0x12, 0x12, 0x12, 0x11, 0x11, 0x11, 0x0f, 0x0f, 0x0f,
			0x0e, 0x0e, 0x0e, 0x0d, 0x0d, 0x0d, 0x0c, 0x0c, 0x0c,
			0x0b, 0x0b, 0x0b, 0x0a, 0x0a, 0x0a, 0x09, 0x09, 0x09,
			0x06, 0x06, 0x06, 0xb0, 0xb0, 0xb0, 0x9c, 0x9c, 0x9c,
			0x82, 0x82, 0x82, 0x6f, 0x6f, 0x6f, 0x64, 0x64, 0x64,
			0x5b, 0x5b, 0x5b, 0x54, 0x54, 0x54, 0x4d, 0x4d, 0x4d,
			0x46, 0x46, 0x46, 0x41, 0x41, 0x41, 0x3a, 0x3a, 0x3a,
			0x33, 0x33, 0x33, 0x2c, 0x2c, 0x2c, 0x23, 0x23, 0x23,
			0x18, 0x18, 0x18, 0x08, 0x08, 0x08, 0x72, 0x46, 0x58,
			0x4c, 0x59, 0x59, 0x6e, 0x81, 0x7e, 0x6b, 0xab, 0x08,
			0x6b, 0xa5, 0x10, 0x7d, 0xa1, 0x08, 0x7b, 0x8c, 0x00,
			0x7b, 0x84, 0x00, 0x61, 0x61, 0x07, 0x66, 0x5b, 0x17,
			0x89, 0x76, 0x20, 0xba, 0xb0, 0x8d, 0x86, 0x67, 0x03,
			0x9b, 0x73, 0x07, 0x9c, 0x63, 0x00, 0x94, 0x5a, 0x00,
			0xfa, 0xa1, 0x29, 0xd6, 0x87, 0x1e, 0xf0, 0x9a, 0x34,
			0xf2, 0x91, 0x20, 0xd7, 0x7b, 0x1a, 0xc0, 0x6d, 0x16,
			0xd6, 0x81, 0x29, 0xce, 0x6d, 0x0d, 0xdd, 0x91, 0x47,
			0xb3, 0x5b, 0x06, 0xf1, 0x8b, 0x29, 0xd6, 0x6e, 0x0d,
			0xec, 0x7f, 0x1e, 0xbd, 0x5c, 0x07, 0xce, 0x6e, 0x1a,
			0xd6, 0x6f, 0x1a, 0xd2, 0x7b, 0x33, 0xe2, 0x70, 0x15,
			0xca, 0x5d, 0x07, 0xb2, 0x52, 0x07, 0xc9, 0x6f, 0x29,
			0x87, 0x63, 0x47, 0xaa, 0x4a, 0x00, 0xb2, 0x56, 0x10,
			0xbd, 0x59, 0x10, 0xb6, 0x5b, 0x18, 0x9a, 0x43, 0x05,
			0xc6, 0x5c, 0x10, 0xbd, 0x52, 0x07, 0xe4, 0x71, 0x21,
			0xaa, 0x4a, 0x08, 0xd2, 0x5e, 0x10, 0xc6, 0x60, 0x1b,
			0xa8, 0x42, 0x00, 0xb2, 0x5d, 0x26, 0x9b, 0x73, 0x59,
			0xd4, 0x61, 0x19, 0xb9, 0x4a, 0x06, 0x8f, 0x49, 0x1e,
			0x88, 0x32, 0x00, 0xaa, 0x4a, 0x13, 0xa8, 0x42, 0x08,
			0x98, 0x36, 0x00, 0x87, 0x33, 0x08, 0xbd, 0x4a, 0x11,
			0x67, 0x22, 0x00, 0x7d, 0x29, 0x00, 0xa8, 0x42, 0x11,
			0x98, 0x36, 0x08, 0xb9, 0x3e, 0x08, 0xa7, 0x36, 0x05,
			0x90, 0x29, 0x00, 0x57, 0x18, 0x00, 0x86, 0x33, 0x13,
			0x44, 0x12, 0x00, 0x73, 0x1e, 0x00, 0x81, 0x20, 0x00,
			0x81, 0x26, 0x08, 0x67, 0x18, 0x00, 0x60, 0x1f, 0x0f,
			0x52, 0x0f, 0x00, 0x5e, 0x10, 0x00, 0x3d, 0x19, 0x14,
			0x45, 0x08, 0x00, 0x35, 0x06, 0x00, 0x22, 0x03, 0x00,
			0xd7, 0x11, 0x10, 0x08, 0x00, 0x00, 0x7b, 0x31, 0x31,
			0xff, 0xff, 0xff
			})));

	_palette = menuPalette; // TODO: move to AssetsManager? save when read the PCX?
}

void fixPidOffset(string pidPath, int32_t& offsetX, int32_t& offsetY)
{
	// hack- todo: edit the files?

	if (pidPath[0] != '/')
	{
		pidPath = "/" + pidPath;
	}

	// NOTE: number-filenames are according to `fixFileName` at `RezArchive.cpp`


	// HUD pistol
	if (pidPath == "/GAME/IMAGES/INTERFACE/WEAPONS/PISTOL/001.PID")
	{
		offsetY += 1;
	}
	// HUD magic
	else if (pidPath == "/GAME/IMAGES/INTERFACE/WEAPONS/MAGIC/001.PID")
	{
		offsetY += 2;
	}
	else if (pidPath == "/LEVEL7/IMAGES/SANDHOLE/005.PID")
	{
		offsetX -= 2;
	}
	// Claw magic
	else if (pidPath == "/CLAW/IMAGES/165.PID")
	{
		offsetX += 15;
	}
	else if (pidPath == "/CLAW/IMAGES/166.PID")
	{
		offsetX += -5;
	}
	else if (pidPath == "/CLAW/IMAGES/167.PID")
	{
		offsetX += 10;
	}
	// Claw swipe
	else if (pidPath == "/CLAW/IMAGES/024.PID" || pidPath == "/CLAW/IMAGES/025.PID" || pidPath == "/CLAW/IMAGES/026.PID" || pidPath == "/CLAW/IMAGES/027.PID")
	{
		offsetX += 22;
	}
	// Cursor
	else if (pidPath == "/GAME/IMAGES/CURSOR/CURSOR04.PID")
	{
		offsetX += 2;
	}
	// Tower Cannon
	if (pidPath == "/LEVEL2/IMAGES/TOWERCANNONLEFT/002.PID" ||
		pidPath == "/LEVEL2/IMAGES/TOWERCANNONLEFT/004.PID" ||
		pidPath == "/LEVEL2/IMAGES/TOWERCANNONLEFT/005.PID")
	{
		offsetX -= 2;
	}
	else if (pidPath == "/LEVEL2/IMAGES/TOWERCANNONRIGHT/002.PID"
		|| pidPath == "/LEVEL2/IMAGES/TOWERCANNONRIGHT/004.PID"
		|| pidPath == "/LEVEL2/IMAGES/TOWERCANNONRIGHT/005.PID")
	{
		offsetX += 2;
	}
	else if (pidPath == "/LEVEL8/IMAGES/CANNONSWITCH/002.PID")
	{
		offsetX -= 2;
		offsetY -= 1;
	}
}
shared_ptr<UIBaseImage> ImagesManager::loadPidImage(const string& pidPath)
{
	//vector<uint8_t> data = rezArchive->GetRezFileData(pidPath);
	//BufferReader pidFileReader(data);

	shared_ptr<BufferReader> pidFileReader = _rezArchive->getFileBufferReader(pidPath);
	vector<ColorRGBA> pixels;
	shared_ptr<PidPalette> imagePalette = _palette;
	uint32_t flags; // PID_FLAG_
	uint32_t width, height, x = 0, y = 0;
	int32_t offsetX, offsetY, i;
	uint8_t byte;

	pidFileReader->skip(4);
	pidFileReader->read(flags);
	pidFileReader->read(width);
	pidFileReader->read(height);
	pidFileReader->read(offsetX);
	pidFileReader->read(offsetY);
	pidFileReader->skip(8);

	fixPidOffset(pidPath, offsetX, offsetY);

	/********************** PID PALETTE **********************/

	const bool pidHasPalette = flags & PidFlag_EmbeddedPalette;

	if (pidHasPalette)
	{
		// if image has embedded palette within it, extract it

		// TODO: checkpoint. if it doesnt used delete it
		// AH: I don't have a file to check this code ... so if we don't use it - delete it

		throw Exception(__FUNCTION__ ": finally i'm here!");
		/*
		uint32_t paletteOffset = (uint32_t)data.size() - PidPalette::PALETTE_SIZE_BYTES;
		const int64_t currPos = pidFileReader.getIndex();
		imagePalette = DBG_NEW PidPalette(pidFileReader.ReadVector(paletteOffset));
		pidFileReader.setIndex(currPos);
		*/
	}

	/********************** PID PIXELS **********************/

	pixels.resize((size_t)width * height);

	// PID is compressed, RLE
	if (flags & PidFlag_Compression)
	{
		while (y < height)
		{
			pidFileReader->read(byte);

			i = byte;

			if (byte > 128)
			{
				i -= 128;
				while (i > 0 && y < height)
				{
					pixels[(size_t)y * width + x] = ColorRGBA{ 0, 0, 0, 1 };
					x++;
					if (x == width)
					{
						x = 0;
						y++;
					}
					i--;
				}
			}
			else
			{
				while (i > 0 && y < height)
				{
					pidFileReader->read(byte);
					pixels[(size_t)y * width + x] = imagePalette->colors[byte];

					x++;
					if (x == width)
					{
						x = 0;
						y++;
					}
					i--;
				}
			}
		}
	}
	else
	{
		while (y < height)
		{
			pidFileReader->read(byte);

			// PID related encoding probably, this means how many same pixels are following.
			// e.g. if byte = 220, then 220-192=28 same pixels are next to each other
			if (byte > 192)
			{
				i = byte - 192;
				pidFileReader->read(byte);
			}
			else
			{
				i = 1;
			}

			while (i > 0 && y < height)
			{
				pixels[(size_t)y * width + x] = imagePalette->colors[byte];

				x++;
				if (x == width)
				{
					x = 0;
					y++;
				}
				i--;
			}
		}
	}

	return allocNewSharedPtr<UIBaseImage>(
		WindowManager::createBitmapFromBuffer(pixels.data(), width, height),
		Point2F((float)offsetX, (float)offsetY));
}
shared_ptr<UIBaseImage> ImagesManager::loadPcxImage(const string& pcxPath)
{
	// based on https://github.com/freudi74/mfimage (I copied only what I need)

	ColorRGBA pcxPalette[256] = {};
	shared_ptr<BufferReader> pcxReader = _rezArchive->getFileBufferReader(pcxPath);
	vector<ColorRGBA> pixels; // list of RGBA pixels
	uint32_t width, height, maxXPadding, plane, x, y;
	uint16_t startX, startY, endX, endY, bytesPerLine;
	uint8_t pcxVer, pcxId, pcxReserved1, numBitPlanes, runCount, runValue, i, byteValue, u8;
	uint8_t pcxEnc; // encoding: 0 = raw; 1 = RLE
	uint8_t bpp; // bits per pixel (or: plane)
	int8_t bit;


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
		pcxReader->read(pcxPalette[i].r);
		pcxReader->read(pcxPalette[i].g);
		pcxReader->read(pcxPalette[i].b);
		pcxPalette[i].a = 255;
	}

	pcxReader->read(pcxReserved1);
	pcxReader->read(numBitPlanes);
	pcxReader->read(bytesPerLine);


	if (pcxId != 0x0a || pcxVer != 5 || pcxVer == 1 || pcxEnc > 1 || endX <= startX || endY <= startY || pcxReserved1 || numBitPlanes != 1)
	{
		throw Exception("not PCX or unsupported version");
	}

	width = endX - startX + 1;
	height = endY - startY + 1;
	pixels.resize((size_t)width * height);
	maxXPadding = bytesPerLine * 8 / bpp;

	pcxReader->skip(60);

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
				pcxReader->read(pcxPalette[i].r);
				pcxReader->read(pcxPalette[i].g);
				pcxReader->read(pcxPalette[i].b);
				pcxPalette[i].a = 255;
			}
		}
		pcxReader->setIndex(endOfHeader);
	}

	pcxPalette[0].a = 0; // first pixel in palette is transparent


	plane = 0;
	x = 0;
	y = 0;
	bit = 0;
	runCount = 0;

	do {
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

				pixels[(size_t)width * y + x++] = pcxPalette[byteValue];
			} while (bit != 0);
		}
	} while (y < height);

	return allocNewSharedPtr<UIBaseImage>(WindowManager::createBitmapFromBuffer(pixels.data(), width, height));
}
