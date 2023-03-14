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


ImagesManager::ImagesManager(RezArchive* rezArchive)
	: _rezArchive(rezArchive)
{
}
ImagesManager::~ImagesManager()
{
	for (auto& i : _loadedBitmaps)
	{
		SafeRelease(&i.second.first);
	}
}

void ImagesManager::setPalette(shared_ptr<PidPalette> palette)
{
	_palette = palette;
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
		if (endsWith(path, ".PID"))
		{
			img = loadPidImage(path);
		}
		else if (endsWith(path, ".PCX"))
		{
			img = loadPcxImage(path);
		}
		else if (path.empty())
		{
			img = allocNewSharedPtr<UIBaseImage>(nullptr); // empty image
		}
		else
		{
			// TODO: insert empty image instead of throw exception ?
			throw Exception(__FUNCTION__ " - not image. path=" + path);
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

	_palette = nullptr;
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
		createBitmapFromBuffer(pixels.data(), width, height),
		Point2F((float)offsetX, (float)offsetY)
		);
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

		if (pcxReader->readByte() == 0x0c)
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
		u8 = pcxReader->readByte();

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

	return allocNewSharedPtr<UIBaseImage>(createBitmapFromBuffer(pixels.data(), width, height));
}

ID2D1Bitmap* ImagesManager::createBitmapFromBuffer(const void* const buffer, uint32_t width, uint32_t height)
{
	ID2D1Bitmap* bitmap = nullptr;
	IWICBitmap* wicBitmap = nullptr;

	HRESULT_THROW_IF_FAILED(WindowManager::_wicImagingFactory->CreateBitmapFromMemory(
		width, height,
		GUID_WICPixelFormat32bppPRGBA,
		width * 4,
		width * height * 4,
		(BYTE*)buffer,
		&wicBitmap));

	HRESULT_THROW_IF_FAILED(WindowManager::_renderTarget->CreateBitmapFromWicBitmap(wicBitmap, &bitmap));

	SafeRelease(&wicBitmap);

	return bitmap;
}
