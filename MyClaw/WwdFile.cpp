#include "WwdFile.h"
#include "AssetsManager.h"
#include "WindowManager.h"
#include "ActionPlane.h"


enum WwdFlags
{
	WwdFlag_UseZCoords = 1 << 0,
	WwdFlag_Compress = 1 << 1,
};
enum WwdPlaneFlags
{
	WwdPlaneFlags_MainPlane = 1 << 0,
	WwdPlaneFlags_NoDraw = 1 << 1,
	WwdPlaneFlags_XWrapping = 1 << 2,
	WwdPlaneFlags_YWrapping = 1 << 3,
	WwdPlaneFlags_AutoTileSize = 1 << 4
};


// impleted in Miniz.cpp
void mz_uncompress(uint8_t pDest[], uint32_t pDestLen, const uint8_t pSource[], uint32_t sourceLen);


WwdObject::WwdObject()
{
	id = 0;
	x = 0;
	y = 0;
	z = 0;
	i = 0;
	addFlags = 0;
	dynamicFlags = 0;
	drawFlags = 0;
	userFlags = 0;
	score = 0;
	points = 0;
	powerup = 0;
	damage = 0;
	smarts = 0;
	health = 0;
	moveRect = {};
	hitRect = {};
	attackRect = {};
	clipRect = {};
	userRect1 = {};
	userRect2 = {};
	userValue1 = 0;
	userValue2 = 0;
	userValue3 = 0;
	userValue4 = 0;
	userValue5 = 0;
	userValue6 = 0;
	userValue7 = 0;
	userValue8 = 0;
	minX = 0;
	minY = 0;
	maxX = 0;
	maxY = 0;
	speedX = 0;
	speedY = 0;
	tweakX = 0;
	tweakY = 0;
	counter = 0;
	speed = 0;
	width = 0;
	height = 0;
	direction = 0;
	faceDir = 0;
	timeDelay = 0;
	frameDelay = 0;
	objectType = 0;
	hitTypeFlags = 0;
	moveResX = 0;
	moveResY = 0;
}

WapWorld::WapWorld(shared_ptr<BufferReader> wwdFileReader, int levelNumber)
	: levelNumber(levelNumber)
{
	wwdFileReader->skip(8);
	uint32_t flags = wwdFileReader->read<uint32_t>();
	
	if (!(flags & WwdFlag_Compress))
		throw Exception("WWD file is not compressed");

	wwdFileReader->skip(4);
	WindowManager::setTitle(wwdFileReader->ReadString(64));
	wwdFileReader->skip(384); // 64 bytes - author, 64 bytes - birth, 256 bytes - rezFile
	string imageDirectoryPath = replaceString(wwdFileReader->ReadString(128), '\\', '/');
	shared_ptr<PidPalette> palette = AssetsManager::loadPidPalette(replaceString(wwdFileReader->ReadString(128), '\\', '/'));

	wwdFileReader->read(startX);
	wwdFileReader->read(startY);
	wwdFileReader->skip(4);
	uint32_t numOfPlanes = wwdFileReader->read<uint32_t>();
	uint32_t planesOffset = wwdFileReader->read< uint32_t>();
	uint32_t tileDescriptionsOffset = wwdFileReader->read<uint32_t>();
	uint32_t mainBlockLength = wwdFileReader->read<uint32_t>();
	wwdFileReader->skip(136); // 4 checksum + 4 unk + 128 launch app

	string prefix[4], imageSet[4];
	for (int8_t i = 0; i < 4; imageSet[i++] = replaceString(wwdFileReader->ReadString(128), '\\', '/'));
	for (int8_t i = 0; i < 4; prefix[i++] = replaceString(wwdFileReader->ReadString(32), '\\', '/'));
	PathManager::setRoots(prefix, imageSet);

	// Uncompressed WWD file payload info
	uint32_t decompressedMainBlockLength = planesOffset + mainBlockLength;
	uint8_t* decompressedMainBlock = DBG_NEW uint8_t[decompressedMainBlockLength];
	memcpy(decompressedMainBlock, wwdFileReader->getCData(), planesOffset);
	mz_uncompress(decompressedMainBlock + planesOffset, mainBlockLength, wwdFileReader->getCData() + planesOffset, (uint32_t)wwdFileReader->getSize() - planesOffset);
	wwdFileReader.reset(); // we don't need it anymore
	BufferReader wwdFileStreamInflated(decompressedMainBlock, decompressedMainBlockLength, false);
	wwdFileStreamInflated.setIndex(planesOffset);
	readPlanes(wwdFileStreamInflated, palette->colors, imageDirectoryPath, numOfPlanes);
	wwdFileStreamInflated.setIndex(tileDescriptionsOffset);
	readTileDescriptions(wwdFileStreamInflated);
	delete[] decompressedMainBlock;


	for (auto& p : planes) p->init();

	if (flags & WwdFlag_UseZCoords)
	{
		sort(planes.begin(), planes.end(), [](const LevelPlane* a, const LevelPlane* b) { return a->ZCoord < b->ZCoord; });
	}

	WindowManager::setBackgroundColor(planes[0]->fillColor);

	// minor change to tiles so they will be more accurate for reduce rectangles (in PhysicsManager)
	// in levels 5,11 it is necessary to change the tiles for "BreakPlanks"
	// TODO: levels 5,11: connect the planks to the tiles
	// TODO: continue for all levels (if needed)
	if (levelNumber == 1)
	{
		WwdTileDescription& t401 = tilesDescription[401];
		WwdTileDescription& t406 = tilesDescription[406];

		t401.rect.right = 28;
		t401.rect.bottom = 63;
		t401.insideAttrib = WwdTileDescription::TileAttribute_Solid;
		t401.type = WwdTileDescription::TileType_Double;

		t406.rect.left = 40;
		t406.rect.right = 63;
		t406.rect.bottom = 63;
		t406.insideAttrib = WwdTileDescription::TileAttribute_Solid;
		t406.type = WwdTileDescription::TileType_Double;
	}
	else if (levelNumber == 2)
	{
		tilesDescription[88].outsideAttrib = WwdTileDescription::TileAttribute_Clear;
		tilesDescription[91].insideAttrib = WwdTileDescription::TileAttribute_Clear;
	}
	else if (levelNumber == 5)
	{
		WwdTileDescription& t509 = tilesDescription[509];
		t509.insideAttrib = WwdTileDescription::TileAttribute_Clear;
		t509.outsideAttrib = WwdTileDescription::TileAttribute_Clear;
		tilesDescription[519].rect.bottom += 2;
	}
	else if (levelNumber == 11)
	{
		WwdTileDescription& t39 = tilesDescription[39];
		t39.insideAttrib = WwdTileDescription::TileAttribute_Clear;
		t39.outsideAttrib = WwdTileDescription::TileAttribute_Clear;
	}
	else if (levelNumber == 14)
	{
		tilesDescription[49].insideAttrib = WwdTileDescription::TileAttribute_Clear;
		tilesDescription[50].insideAttrib = WwdTileDescription::TileAttribute_Clear;
		tilesDescription[54].insideAttrib = WwdTileDescription::TileAttribute_Clear;
	}

	tilesDescription.clear();
}
void WapWorld::readPlanes(BufferReader& reader, const ColorRGBA colors[], const string& imageDirectoryPath, uint32_t numOfPlanes)
{
	vector<string> imageSets;
	int64_t currIdx;
	uint32_t fillColor, tilesOffset, imageSetsOffset, objectsOffset, flags, tilePixelWidth, tilePixelHeight;

	while (numOfPlanes-- > 0)
	{
#ifdef LOW_DETAILS
		reader.skip(16); // skip to plane name
		// is case of low-details we pass the front plane
		if (reader.ReadString(64) == "Front")
		{
			reader.skip(76); // skip the rest of the plane
			continue;
		}
		// back to plane start
		reader.skip(-80);
#endif

		LevelPlane* pln;
		
		reader.skip(8);
		reader.read(flags);

		if (flags & WwdPlaneFlags_MainPlane)
		{
			pln = DBG_NEW ActionPlane(this, levelNumber);
			pln->_isMainPlane = true;
		}
		else
		{
			pln = DBG_NEW LevelPlane();
		}

		reader.skip(76); // 4 unk, 64 plane-name, 4 total pixel width, 4 total pixel height
		reader.read(tilePixelWidth);
		reader.read(tilePixelHeight);

		if (tilePixelWidth != TILE_SIZE && tilePixelHeight != TILE_SIZE)
			throw Exception("invalid size of tile");

		reader.read(pln->tilesOnAxisX);
		reader.read(pln->tilesOnAxisY);
		reader.skip(8);
		pln->movementPercentX = reader.read<uint32_t>() / 100.f;
		pln->movementPercentY = reader.read<uint32_t>() / 100.f;
		reader.read(fillColor);
		imageSets.resize(reader.read<uint32_t>());
		pln->objects.resize(reader.read<uint32_t>());
		reader.read(tilesOffset);
		reader.read(imageSetsOffset);
		reader.read(objectsOffset);
		reader.read(pln->ZCoord);

		currIdx = reader.getIndex() + 12; // save the current index and skip 12 bytes

		// read tiles
		reader.setIndex(tilesOffset);
		pln->tiles.resize(pln->tilesOnAxisY);
		for (vector<int32_t>& vec : pln->tiles)
		{
			vec.resize(pln->tilesOnAxisX);
			for (int32_t& i : vec)
			{
				reader.read(i);
				if (i < 0) i = -1;
			}
		}

		// read image sets
		reader.setIndex(imageSetsOffset);
		for (string& s : imageSets) s = reader.ReadNullTerminatedString();

		// read objects
		reader.setIndex(objectsOffset);
		pln->readPlaneObjects(reader);

		reader.setIndex(currIdx);

		// set plane properties. used when drawing the plane
		pln->maxTileIdxX = ((flags & WwdPlaneFlags_XWrapping) ? INT32_MAX : pln->tilesOnAxisX);
		pln->maxTileIdxY = ((flags & WwdPlaneFlags_YWrapping) ? INT32_MAX : pln->tilesOnAxisY);

		pln->fillColor = ColorF(colors[fillColor].r / 255.f, colors[fillColor].g / 255.f, colors[fillColor].b / 255.f);
		pln->tilesImages = AssetsManager::loadPlaneTilesImages(imageDirectoryPath + '/' + imageSets[0]);

		imageSets.clear();

		planes.push_back(pln);
	}
}

void WapWorld::readTileDescriptions(BufferReader& reader)
{
	reader.skip(8);
	vector<WwdTileDescription> tilesDescList(reader.read<uint32_t>());
	reader.skip(20);

	uint32_t width, height; // in pixels

	for (WwdTileDescription& tileDesc : tilesDescList)
	{
		reader.read(tileDesc.type);
		reader.skip(4);
		reader.read(width);
		reader.read(height);

		if (width != TILE_SIZE && height != TILE_SIZE)
			throw Exception("invalid size");

		switch (tileDesc.type)
		{
		case WwdTileDescription::TileType_Single:
			reader.read(tileDesc.insideAttrib);
			break;

		case WwdTileDescription::TileType_Double:
			reader.read(tileDesc.outsideAttrib);
			reader.read(tileDesc.insideAttrib);
			reader.read(tileDesc.rect);
			if (tileDesc.rect.left   < 0 || TILE_SIZE < tileDesc.rect.left  ||
				tileDesc.rect.top    < 0 || TILE_SIZE < tileDesc.rect.top   ||
				tileDesc.rect.right  < 0 || TILE_SIZE < tileDesc.rect.right ||
				tileDesc.rect.bottom < 0 || TILE_SIZE < tileDesc.rect.bottom)
				throw Exception("invalid rect");
			break;

		default:
			throw Exception("invalid type");
		}
	}

	// save only what we need...
	tilesDescription[-1].type = WwdTileDescription::TileType_Empty;
	for (LevelPlane* pln : planes)
		if (pln->_isMainPlane)
		{
			for (const vector<int32_t>& vec : pln->tiles)
				for (const int32_t& tileId : vec)
					if (tilesDescription.count(tileId) == 0)
						tilesDescription[tileId] = tilesDescList[tileId];
			break;
		}
}
