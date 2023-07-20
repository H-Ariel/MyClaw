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


// Single-call decompression.
// Returns MZ_OK on success, or one of the error codes from mz_inflate() on failure.
// impleted in Miniz.cpp
void mz_uncompress(uint8_t* pDest, uint32_t* pDestLen, const uint8_t* pSource, uint32_t source_len);


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

WwdPlaneData::WwdPlaneData()
	: fillColor(0), tilesOnAxisX(0), tilesOnAxisY(0),
	movementPercentX(0), movementPercentY(0), ZCoord(0),
	isWrappedX(false), isWrappedY(false), isMainPlane(false) {}

WapWorld::WapWorld(shared_ptr<BufferReader> wwdFileReader, int levelNumber)
{
	// signature holds WWD header size, if size does not match then it is not supported WWD file
	if (wwdFileReader->read<uint32_t>() != 1524)
	{
		throw Exception("invlaid WWD file");
	}

	wwdFileReader->skip(4);
	uint32_t flags = wwdFileReader->read<uint32_t>();
	wwdFileReader->skip(4);
	WindowManager::setTitle(wwdFileReader->ReadString(64));
	wwdFileReader->skip(384); // 64 bytes - author, 64 bytes - birth, 256 bytes - rezFile
	string imageDirectoryPath = replaceString(wwdFileReader->ReadString(128), '\\', '/');
	shared_ptr<PidPalette> palette = AssetsManager::loadPidPalette(replaceString(wwdFileReader->ReadString(128), '\\', '/'));

	wwdFileReader->read(startX);
	wwdFileReader->read(startY);
	wwdFileReader->skip(4);
	vector<WwdPlaneData> planesData(wwdFileReader->read<uint32_t>());
	uint32_t planesOffset = wwdFileReader->read< uint32_t>();
	uint32_t tileDescriptionsOffset = wwdFileReader->read<uint32_t>();
	uint32_t mainBlockLength = wwdFileReader->read<uint32_t>();
	wwdFileReader->skip(136); // 4 checksum + 4 unk + 128 launch app

	string prefix[4], imageSet[4];
	for (int8_t i = 0; i < 4; imageSet[i++] = replaceString(wwdFileReader->ReadString(128), '\\', '/'));
	for (int8_t i = 0; i < 4; prefix[i++] = replaceString(wwdFileReader->ReadString(32), '\\', '/'));
	PathManager::setRoots(prefix, imageSet);

	if (flags & WwdFlag_Compress)
	{
		// Uncompressed WWD file payload info
		uint32_t decompressedMainBlockLength = planesOffset + mainBlockLength;
		uint8_t* decompressedMainBlock = DBG_NEW uint8_t[decompressedMainBlockLength];
		memcpy(decompressedMainBlock, wwdFileReader->getCData(), planesOffset);

		// Inflate compressed WWD file payload
		mz_uncompress(decompressedMainBlock + planesOffset, &mainBlockLength, wwdFileReader->getCData() + planesOffset, (uint32_t)wwdFileReader->getSize() - planesOffset);

		BufferReader wwdFileStreamInflated(decompressedMainBlock, decompressedMainBlockLength, false);
		wwdFileStreamInflated.setIndex(planesOffset);
		readPlanes(wwdFileStreamInflated, planesData, palette->colors, imageDirectoryPath);
		wwdFileStreamInflated.setIndex(tileDescriptionsOffset);
		readTileDescriptions(wwdFileStreamInflated, planesData);

		delete[] decompressedMainBlock;
	}
	else
	{
		throw Exception("WWD file is not compressed");
	}

	if (flags & WwdFlag_UseZCoords)
	{
		sort(planesData.begin(), planesData.end(), [](const WwdPlaneData& a, const WwdPlaneData& b) { return a.ZCoord < b.ZCoord; });
	}

	WindowManager::setBackgroundColor(planesData[0].fillColor);
	
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

	for (const WwdPlaneData& pln : planesData)
	{
		if (pln.isMainPlane)
		{
			planes.push_back(DBG_NEW ActionPlane(pln, this, levelNumber));
		}
		else
		{
#ifdef LOW_DETAILS
			if (pln.name == "Front") continue;
#endif
			planes.push_back(DBG_NEW LevelPlane(pln));
		}
	}

	tilesDescription.clear();
}
void WapWorld::readPlanes(BufferReader& reader, vector<WwdPlaneData>& planesData, const ColorRGBA colors[], const string& imageDirectoryPath)
{
	vector<string> imageSets;
	int64_t currIdx;
	uint32_t fillColor;
	uint32_t tilesOffset, imageSetsOffset, objectsOffset;
	uint32_t flags; // WwdPlaneFlags flags

	uint32_t tilePixelWidth, tilePixelHeight; // in pixels

	for (WwdPlaneData& pln : planesData)
	{
		reader.skip(8);
		reader.read(flags);
		reader.skip(4);
		pln.name = reader.ReadString(64);
		reader.skip(8); // total pixel width, total pixel height
		reader.read(tilePixelWidth);
		reader.read(tilePixelHeight);

		if (tilePixelWidth != TILE_SIZE && tilePixelHeight != TILE_SIZE)
			throw Exception("invalid size of tile");

		reader.read(pln.tilesOnAxisX);
		reader.read(pln.tilesOnAxisY);
		reader.skip(8);
		pln.movementPercentX = reader.read<uint32_t>() / 100.f;
		pln.movementPercentY = reader.read<uint32_t>() / 100.f;
		reader.read(fillColor);
		imageSets.resize(reader.read<uint32_t>());
		pln.objects.resize(reader.read<uint32_t>());
		reader.read(tilesOffset);
		reader.read(imageSetsOffset);
		reader.read(objectsOffset);
		reader.read(pln.ZCoord);

		currIdx = reader.getIndex() + 12; // save the current index and skip 12 bytes

		// read tiles
		reader.setIndex(tilesOffset);
		pln.tiles.resize(pln.tilesOnAxisY);
		for (vector<int32_t>& vec : pln.tiles)
		{
			vec.resize(pln.tilesOnAxisX);
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
		readPlaneObjects(reader, pln);

		reader.setIndex(currIdx);

		pln.isWrappedX = flags & WwdPlaneFlags_XWrapping;
		pln.isWrappedY = flags & WwdPlaneFlags_YWrapping;
		pln.isMainPlane = flags & WwdPlaneFlags_MainPlane;
		pln.fillColor = ColorF(colors[fillColor].r / 255.f, colors[fillColor].g / 255.f, colors[fillColor].b / 255.f);
		pln.tilesImages = AssetsManager::loadPlaneTilesImages(imageDirectoryPath + '/' + imageSets[0]);
		
		imageSets.clear();
	}
}
void WapWorld::readPlaneObjects(BufferReader& reader, WwdPlaneData& pln)
{
	uint32_t nameLength, logicLength, imageSetLength, animationLength;

	for (WwdObject& obj : pln.objects)
	{
		reader.read(obj.id);
		reader.read(nameLength);
		reader.read(logicLength);
		reader.read(imageSetLength);
		reader.read(animationLength);
		reader.read(obj.x);
		reader.read(obj.y);
		reader.read(obj.z);
		reader.read(obj.i);
		reader.read(obj.addFlags);
		reader.read(obj.dynamicFlags);
		reader.read(obj.drawFlags);
		reader.read(obj.userFlags);
		reader.read(obj.score);
		reader.read(obj.points);
		reader.read(obj.powerup);
		reader.read(obj.damage);
		reader.read(obj.smarts);
		reader.read(obj.health);

		reader.read(obj.moveRect);
		reader.read(obj.hitRect);
		reader.read(obj.attackRect);
		reader.read(obj.clipRect);
		reader.read(obj.userRect1);
		reader.read(obj.userRect2);

		reader.read(obj.userValue1);
		reader.read(obj.userValue2);
		reader.read(obj.userValue3);
		reader.read(obj.userValue4);
		reader.read(obj.userValue5);
		reader.read(obj.userValue6);
		reader.read(obj.userValue7);
		reader.read(obj.userValue8);
		reader.read(obj.minX);
		reader.read(obj.minY);
		reader.read(obj.maxX);
		reader.read(obj.maxY);
		reader.read(obj.speedX);
		reader.read(obj.speedY);
		reader.read(obj.tweakX);
		reader.read(obj.tweakY);
		reader.read(obj.counter);
		reader.read(obj.speed);
		reader.read(obj.width);
		reader.read(obj.height);
		reader.read(obj.direction);
		reader.read(obj.faceDir);
		reader.read(obj.timeDelay);
		reader.read(obj.frameDelay);
		reader.read(obj.objectType);
		reader.read(obj.hitTypeFlags);
		reader.read(obj.moveResX);
		reader.read(obj.moveResY);

		obj.name = reader.ReadString(nameLength);
		obj.logic = reader.ReadString(logicLength);
		obj.imageSet = reader.ReadString(imageSetLength);
		obj.animation = reader.ReadString(animationLength);
	}

	sort(pln.objects.begin(), pln.objects.end(), [](const WwdObject& a, const WwdObject& b) { return a.z < b.z; });
}
void WapWorld::readTileDescriptions(BufferReader& reader, vector<WwdPlaneData>& planesData)
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
			if (tileDesc.rect.left < 0 || TILE_SIZE < tileDesc.rect.left ||
				tileDesc.rect.top < 0 || TILE_SIZE < tileDesc.rect.top ||
				tileDesc.rect.right < 0 || TILE_SIZE < tileDesc.rect.right ||
				tileDesc.rect.bottom < 0 || TILE_SIZE < tileDesc.rect.bottom)
				throw Exception("invalid rect");
			break;

		default:
			throw Exception("invalid type");
		}
	}

	// save only what we need...
	tilesDescription[-1].type = WwdTileDescription::TileType_Empty;
	for (const WwdPlaneData& pln : planesData)
		if (pln.isMainPlane)
		{
			for (const vector<int32_t>& vec : pln.tiles)
				for (const int32_t& tileId : vec)
					if (tilesDescription.count(tileId) == 0)
						tilesDescription[tileId] = tilesDescList[tileId];
			break;
		}
}
