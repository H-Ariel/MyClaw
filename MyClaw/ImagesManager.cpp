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
		SafeRelease(&i.second.first);
	}
}

shared_ptr<UIBaseImage> ImagesManager::loadImage(string path)
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
		catch (Exception& ex)
		{
			img = allocNewSharedPtr<UIBaseImage>(nullptr); // empty image
			cout << __FUNCTION__ ": WARNING: a blank image has been inserted. image path = \"" << path << '"' << endl;
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

//	_palette = nullptr; // TODO: un comment this? load menu palette?

	{
		// the palette from "STATES/BOOTY/SCREENS/001MAP.PCX"

		vector<uint8_t> pal1 = {
		0, 0, 0,
		255, 252, 202,
		240, 238, 90,
		223, 220, 76,
		219, 217, 74,
		210, 208, 72,
		189, 187, 63,
		154, 153, 52,
		130, 115, 44,
		98, 97, 33,
		64, 63, 21,
		41, 41, 14,
		38, 35, 44,
		13, 11, 113,
		19, 19, 170,
		0, 104, 144,
		178, 187, 188,
		18, 157, 18,
		75, 74, 68,
		112, 111, 105,
		220, 96, 0,
		99, 4, 0,
		138, 138, 138,
		124, 124, 124,
		44, 44, 44,
		0, 0, 0,
		253, 247, 202,
		254, 244, 145,
		252, 236, 67,
		247, 232, 11,
		240, 212, 95,
		224, 183, 31,
		205, 156, 15,
		190, 134, 18,
		250, 238, 140,
		239, 222, 97,
		217, 201, 90,
		194, 179, 78,
		161, 149, 65,
		129, 119, 52,
		91, 0, 18,
		251, 221, 29,
		247, 227, 121,
		219, 136, 15,
		171, 73, 9,
		125, 38, 17,
		219, 217, 254,
		143, 137, 251,
		45, 40, 218,
		7, 6, 182,
		172, 251, 203,
		21, 233, 60,
		0, 158, 0,
		0, 48, 0,
		243, 199, 254,
		202, 81, 234,
		143, 17, 184,
		148, 0, 194,
		254, 193, 194,
		253, 56, 57,
		247, 2, 3,
		146, 11, 24,
		250, 222, 109,
		251, 239, 191,
		242, 198, 81,
		243, 187, 48,
		205, 138, 15,
		232, 152, 13,
		157, 101, 24,
		221, 150, 53,
		199, 138, 57,
		196, 112, 25,
		143, 67, 10,
		79, 36, 5,
		0, 255, 156,
		8, 48, 121,
		255, 0, 255,
		90, 10, 59,
		250, 234, 152,
		241, 188, 0,
		238, 189, 37,
		209, 117, 5,
		145, 72, 6,
		11, 174, 11,
		127, 211, 129,
		112, 50, 3,
		9, 86, 0,
		70, 208, 46,
		222, 18, 255,
		232, 251, 212,
		86, 219, 212,
		0, 142, 203,
		34, 96, 250,
		18, 100, 28,
		55, 157, 56,
		7, 52, 3,
		120, 208, 110,
		192, 109, 134,
		197, 45, 93,
		145, 18, 58,
		95, 64, 28,
		110, 74, 32,
		18, 8, 43,
		41, 0, 0,
		106, 57, 8,
		0, 0, 0,
		115, 99, 49,
		115, 90, 49,
		115, 90, 41,
		115, 82, 33,
		115, 75, 24,
		114, 75, 16,
		112, 66, 14,
		109, 57, 16,
		108, 57, 7,
		99, 57, 7,
		108, 49, 9,
		99, 49, 16,
		99, 49, 8,
		100, 49, 0,
		90, 49, 5,
		100, 41, 5,
		90, 41, 8,
		90, 41, 0,
		81, 42, 8,
		82, 41, 0,
		91, 32, 2,
		80, 33, 8,
		82, 33, 0,
		74, 33, 0,
		65, 34, 3,
		82, 24, 0,
		79, 24, 8,
		74, 24, 0,
		65, 24, 0,
		75, 16, 0,
		61, 16, 0,
		43, 15, 0,
		42, 42, 42,
		35, 35, 35,
		29, 29, 29,
		25, 25, 25,
		23, 23, 23,
		21, 21, 21,
		18, 18, 18,
		17, 17, 17,
		15, 15, 15,
		14, 14, 14,
		13, 13, 13,
		12, 12, 12,
		11, 11, 11,
		10, 10, 10,
		9, 9, 9,
		6, 6, 6,
		176, 176, 176,
		156, 156, 156,
		130, 130, 130,
		111, 111, 111,
		100, 100, 100,
		91, 91, 91,
		84, 84, 84,
		77, 77, 77,
		70, 70, 70,
		65, 65, 65,
		58, 58, 58,
		51, 51, 51,
		44, 44, 44,
		35, 35, 35,
		24, 24, 24,
		8, 8, 8,
		114, 70, 88,
		76, 89, 89,
		110, 129, 126,
		107, 171, 8,
		107, 165, 16,
		125, 161, 8,
		123, 140, 0,
		123, 132, 0,
		97, 97, 7,
		102, 91, 23,
		137, 118, 32,
		186, 176, 141,
		134, 103, 3,
		155, 115, 7,
		156, 99, 0,
		148, 90, 0,
		250, 161, 41,
		214, 135, 30,
		240, 154, 52,
		242, 145, 32,
		215, 123, 26,
		192, 109, 22,
		214, 129, 41,
		206, 109, 13,
		221, 145, 71,
		179, 91, 6,
		241, 139, 41,
		214, 110, 13,
		236, 127, 30,
		189, 92, 7,
		206, 110, 26,
		214, 111, 26,
		210, 123, 51,
		226, 112, 21,
		202, 93, 7,
		178, 82, 7,
		201, 111, 41,
		135, 99, 71,
		170, 74, 0,
		178, 86, 16,
		189, 89, 16,
		182, 91, 24,
		154, 67, 5,
		198, 92, 16,
		189, 82, 7,
		228, 113, 33,
		170, 74, 8,
		210, 94, 16,
		198, 96, 27,
		168, 66, 0,
		178, 93, 38,
		155, 115, 89,
		212, 97, 25,
		185, 74, 6,
		143, 73, 30,
		136, 50, 0,
		170, 74, 19,
		168, 66, 8,
		152, 54, 0,
		135, 51, 8,
		189, 74, 17,
		103, 34, 0,
		125, 41, 0,
		168, 66, 17,
		152, 54, 8,
		185, 62, 8,
		167, 54, 5,
		144, 41, 0,
		87, 24, 0,
		134, 51, 19,
		68, 18, 0,
		115, 30, 0,
		129, 32, 0,
		129, 38, 8,
		103, 24, 0,
		96, 31, 15,
		82, 15, 0,
		94, 16, 0,
		61, 25, 20,
		69, 8, 0,
		53, 6, 0,
		34, 3, 0,
		215, 17, 16,
		8, 0, 0,
		123, 49, 49,
		255, 255, 255
		};

		_palette = allocNewSharedPtr<PidPalette>(pal1);
	}
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

	THROW_IF_NULL(imagePalette);

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
			static int q = 0;

		//	cout << "vector<uint8_t> pal" << q++ << " = {\n";
			// 256 color palette
			for (uint16_t i = 0; i < 256; i++)
			{
				pcxReader->read(pcxPalette[i].r);
				pcxReader->read(pcxPalette[i].g);
				pcxReader->read(pcxPalette[i].b);
				pcxPalette[i].a = 255;

		//		cout << (int)pcxPalette[i].r << ", " << (int)pcxPalette[i].g << ", " << (int)pcxPalette[i].b << ",\n";
			}
		//	cout << "};\n\n";
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
