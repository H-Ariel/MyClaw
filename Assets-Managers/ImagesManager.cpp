#include "ImagesManager.h"
#include "GameEngine/WindowManager.h"


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
			else throw Exception("empty imgae");
		}
		catch (const Exception&)
		{
			img = allocNewSharedPtr<UIBaseImage>(nullptr); // empty image
			DBG_PRINT("WARNING: a blank image has been inserted. image path: \"%s\"\n", path.c_str());
		}

		_loadedBitmaps[path] = { img->_bitmap, img->offset };
	}

	return img;
}
void ImagesManager::clearLevelImages(const string& prefix)
{
	for (auto it = _loadedBitmaps.begin(); it != _loadedBitmaps.end();)
	{
		if (startsWith(it->first, prefix))
			it = _loadedBitmaps.erase(it);
		else
			++it;
	}
}

void fixPidOffset(string pidPath, int32_t& offsetX, int32_t& offsetY)
{
	// TODO: hack - edit the files?
	// NOTE: number-filenames are according to `fixFileName` at `RezArchive.cpp`


	// HUD pistol
	if (pidPath == "GAME/IMAGES/INTERFACE/WEAPONS/PISTOL/001.PID")
	{
		offsetY += 1;
	}
	// HUD magic
	else if (pidPath == "GAME/IMAGES/INTERFACE/WEAPONS/MAGIC/001.PID")
	{
		offsetY += 2;
	}
	else if (pidPath == "LEVEL7/IMAGES/SANDHOLE/005.PID")
	{
		offsetX -= 2;
	}
	// MagicClaw
	else if (pidPath == "CLAW/IMAGES/165.PID")
	{
		offsetX += 15;
	}
	else if (pidPath == "CLAW/IMAGES/166.PID")
	{
		offsetX += -5;
	}
	else if (pidPath == "CLAW/IMAGES/167.PID")
	{
		offsetX += 10;
	}
	// Claw swipe
	else if (pidPath == "CLAW/IMAGES/024.PID" || pidPath == "CLAW/IMAGES/025.PID" ||
		pidPath == "CLAW/IMAGES/026.PID" || pidPath == "CLAW/IMAGES/027.PID")
	{
		offsetX += 22;
	}
	// Cursor
	else if (pidPath == "GAME/IMAGES/CURSOR/004.PID")
	{
		offsetX += 2;
	}
	// Tower Cannon
	else if (pidPath == "LEVEL2/IMAGES/TOWERCANNONLEFT/002.PID" ||
		pidPath == "LEVEL2/IMAGES/TOWERCANNONLEFT/004.PID" ||
		pidPath == "LEVEL2/IMAGES/TOWERCANNONLEFT/005.PID")
	{
		offsetX -= 2;
	}
	else if (pidPath == "LEVEL2/IMAGES/TOWERCANNONRIGHT/002.PID"
		|| pidPath == "LEVEL2/IMAGES/TOWERCANNONRIGHT/004.PID"
		|| pidPath == "LEVEL2/IMAGES/TOWERCANNONRIGHT/005.PID")
	{
		offsetX += 2;
	}
	else if (pidPath == "LEVEL8/IMAGES/CANNONSWITCH/002.PID")
	{
		offsetX -= 2;
		offsetY -= 1;
	}
	// shooters from level 14
	else if (pidPath == "LEVEL14/IMAGES/SHOOTERS/LAUNCHLEFT/002.PID")
	{
		offsetX -= 2;
	}
	else if (pidPath == "LEVEL14/IMAGES/SHOOTERS/LAUNCHLEFT/003.PID")
	{
		offsetX -= 2;
	}
	else if (pidPath == "LEVEL14/IMAGES/SHOOTERS/LAUNCHLEFT/004.PID")
	{
		offsetX -= 2;
	}
	else if (pidPath == "LEVEL14/IMAGES/SHOOTERS/LAUNCHLEFT/005.PID")
	{
		offsetX -= 2;
	}
	else if (pidPath == "LEVEL14/IMAGES/SHOOTERS/LAUNCHLEFT/007.PID")
	{
		offsetX -= 2;
	}
	else if (pidPath == "LEVEL14/IMAGES/SHOOTERS/LAUNCHRIGHT/002.PID")
	{
		offsetX += 4;
	}
	else if (pidPath == "LEVEL14/IMAGES/SHOOTERS/LAUNCHRIGHT/003.PID")
	{
		offsetX += 2;
	}
	else if (pidPath == "LEVEL14/IMAGES/SHOOTERS/LAUNCHRIGHT/004.PID")
	{
		offsetX += 2;
	}
	else if (pidPath == "LEVEL14/IMAGES/SHOOTERS/LAUNCHRIGHT/005.PID")
	{
		offsetX += 2;
	}
	else if (pidPath == "LEVEL14/IMAGES/SHOOTERS/LAUNCHRIGHT/006.PID")
	{
		offsetX += 2;
	}
	else if (pidPath == "LEVEL14/IMAGES/SHOOTERS/LAUNCHRIGHT/007.PID")
	{
		offsetX += 5;
	}
	else if (pidPath == "LEVEL14/IMAGES/SHOOTERS/LAUNCHRIGHT/008.PID")
	{
		offsetX += 3;
	}
}
shared_ptr<UIBaseImage> ImagesManager::loadPidImage(const string& pidPath)
{
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
	uint32_t width, height, maxXPadding, plane = 0, x = 0, y = 0;
	uint16_t startX, startY, endX, endY, bytesPerLine;
	uint8_t pcxVer, pcxId, pcxReserved1, numBitPlanes, runCount=0, runValue, i, byteValue, u8;
	uint8_t pcxEnc; // encoding: 0 = raw; 1 = RLE
	uint8_t bpp; // bits per pixel (or: plane)
	int8_t bit=0;


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
	pcxReader->skip(60);


	if (pcxId != 0x0a || pcxVer != 5 || pcxVer == 1 || pcxEnc > 1 || endX <= startX || endY <= startY || pcxReserved1 || numBitPlanes != 1)
	{
		throw Exception("not PCX or unsupported version");
	}

	width = endX - startX + 1;
	height = endY - startY + 1;
	pixels.resize((size_t)width * height);
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
				pcxReader->read(pcxPalette[i].r);
				pcxReader->read(pcxPalette[i].g);
				pcxReader->read(pcxPalette[i].b);
				pcxPalette[i].a = 255;
			}
		}
		pcxReader->setIndex(endOfHeader);
	}

	pcxPalette[0].a = 0; // first pixel in palette is transparent


	if (startsWith(pcxPath, "STATES/BOOTY/SCREENS"))
	{
		// save pallete at end of level for map and score images
		_palette = allocNewSharedPtr<PidPalette>(pcxPalette);
	}


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

				pixels[(size_t)width * y + x++] = pcxPalette[byteValue];
			} while (bit != 0);
		}
	}

	return allocNewSharedPtr<UIBaseImage>(WindowManager::createBitmapFromBuffer(pixels.data(), width, height));
}
