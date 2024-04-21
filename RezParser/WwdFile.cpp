#include "WwdFile.h"
#include "Miniz.h"


enum WwdFlags
{
	WwdFlag_UseZCoords = 1 << 0,
	WwdFlag_Compress = 1 << 1,
};


const uint32_t EXPECTED_HEADER_SIZE = 1524;

static void ReadRect(BufferReader& stream, WwdRect& rect)
{
	stream.read(rect.left);
	stream.read(rect.top);
	stream.read(rect.right);
	stream.read(rect.bottom);
}

WwdPlane::WwdPlane()
	: tilesWide(0), tilesHigh(0),
	flags(0),
	pixelWidth(0),
	pixelHeight(0),
	tilePixelWidth(0),
	tilePixelHeight(0),
	tilesOnAxisX(0),
	tilesOnAxisY(0),
	movementPercentX(0),
	movementPercentY(0),
	fillColor(0),
	coordZ(0)
{
}

void WwdPlane::ReadPlaneTiles(BufferReader& inputStream)
{
	tiles.resize(tilesOnAxisY);
	for (vector<int32_t>& vec : tiles)
	{
		vec.resize(tilesOnAxisX);
		for (int32_t& i : vec)
		{
			inputStream.read(i);
			if (i < 0) i = -1;
		}
	}
}
void WwdPlane::ReadPlaneImageSets(BufferReader& inputStream)
{
	for (string& s : imageSets)
		s = inputStream.ReadNullTerminatedString();
}
void WwdPlane::ReadPlaneObjects(BufferReader& inputStream)
{
	uint32_t nameLength, logicLength, imageSetLength, animationLength;

	for (WwdObject& obj : objects)
	{
		inputStream.read(obj.id);
		inputStream.read(nameLength);
		inputStream.read(logicLength);
		inputStream.read(imageSetLength);
		inputStream.read(animationLength);
		inputStream.read(obj.x);
		inputStream.read(obj.y);
		inputStream.read(obj.z);
		inputStream.read(obj.i);
		inputStream.read(obj.addFlags);
		inputStream.read(obj.dynamicFlags);
		inputStream.read(obj.drawFlags);
		inputStream.read(obj.userFlags);
		inputStream.read(obj.score);
		inputStream.read(obj.points);
		inputStream.read(obj.powerup);
		inputStream.read(obj.damage);
		inputStream.read(obj.smarts);
		inputStream.read(obj.health);
		ReadRect(inputStream, obj.moveRect);
		ReadRect(inputStream, obj.hitRect);
		ReadRect(inputStream, obj.attackRect);
		ReadRect(inputStream, obj.clipRect);
		ReadRect(inputStream, obj.userRect1);
		ReadRect(inputStream, obj.userRect2);
		inputStream.read(obj.userValue1);
		inputStream.read(obj.userValue2);
		inputStream.read(obj.userValue3);
		inputStream.read(obj.userValue4);
		inputStream.read(obj.userValue5);
		inputStream.read(obj.userValue6);
		inputStream.read(obj.userValue7);
		inputStream.read(obj.userValue8);
		inputStream.read(obj.minX);
		inputStream.read(obj.minY);
		inputStream.read(obj.maxX);
		inputStream.read(obj.maxY);
		inputStream.read(obj.speedX);
		inputStream.read(obj.speedY);
		inputStream.read(obj.tweakX);
		inputStream.read(obj.tweakY);
		inputStream.read(obj.counter);
		inputStream.read(obj.speed);
		inputStream.read(obj.width);
		inputStream.read(obj.height);
		inputStream.read(obj.direction);
		inputStream.read(obj.faceDir);
		inputStream.read(obj.timeDelay);
		inputStream.read(obj.frameDelay);
		inputStream.read(obj.objectType);
		inputStream.read(obj.hitTypeFlags);
		inputStream.read(obj.moveResX);
		inputStream.read(obj.moveResY);

		obj.name = inputStream.ReadString(nameLength);
		obj.logic = inputStream.ReadString(logicLength);
		obj.imageSet = inputStream.ReadString(imageSetLength);
		obj.animation = inputStream.ReadString(animationLength);
	}
}

WapWwd::WapWwd(shared_ptr<BufferReader> wwdReader, int levelNumber)
	: levelNumber(levelNumber)
{
	uint32_t wwdSignature;

	wwdReader->read(wwdSignature);
	// Signature holds WWD header size, if size doesnt match then it is not supported WWD file
	if (wwdSignature != EXPECTED_HEADER_SIZE)
		throw Exception("Unsupported WWD file");

	uint32_t flags; // WwdFlags
	uint32_t numPlanes;
	uint32_t planesOffset;
	uint32_t tileDescriptionsOffset;
	uint32_t mainBlockLength;

	char author[64];
	char birth[64];
	char rezFile[256];
	char launchApp[128];


	wwdReader->skip(4);
	wwdReader->read(flags);
	wwdReader->skip(4);
	levelName = wwdReader->ReadString(64);
	wwdReader->read(author);
	wwdReader->read(birth);
	wwdReader->read(rezFile);
	imageDirectoryPath = wwdReader->ReadString(128);
	rezPalettePath = wwdReader->ReadString(128);
	wwdReader->read(startX);
	wwdReader->read(startY);
	wwdReader->skip(4);
	wwdReader->read(numPlanes);
	wwdReader->read(planesOffset);
	wwdReader->read(tileDescriptionsOffset);
	wwdReader->read(mainBlockLength);
	wwdReader->skip(8);
	wwdReader->read(launchApp);

	for (int8_t i = 0; i < 4; imageSet[i++] = wwdReader->ReadString(128));
	for (int8_t i = 0; i < 4; prefix[i++] = wwdReader->ReadString(32));

	if (!(flags & WwdFlag_Compress))
		throw Exception("WWD file is not compressed");

	vector<uint8_t> cdata = wwdReader->getData();
	uint8_t* data = cdata.data();
	uint32_t length = (uint32_t)cdata.size();

	// Compressed WWD file payload info
	const uint8_t* compressedMainBlock = data + planesOffset;
	uint32_t compressedMainBlockSize = length - planesOffset;

	// Uncompressed WWD file payload info
	uint32_t decompressedMainBlockVectorLength = planesOffset + mainBlockLength;
	uint8_t* decompressedMainBlockVector = new uint8_t[decompressedMainBlockVectorLength];
	memcpy(decompressedMainBlockVector, data, planesOffset);
	uint8_t* decompressedMainBlock = decompressedMainBlockVector + planesOffset;

	// Inflate compressed WWD file payload
	mz_uncompress(decompressedMainBlock, mainBlockLength, compressedMainBlock, compressedMainBlockSize);

	// Create new file stream from inflated WWD file payload
	BufferReader wwdFileStreamInflated(decompressedMainBlockVector, decompressedMainBlockVectorLength, false);

	// read level informations
	wwdFileStreamInflated.setIndex(planesOffset);
	planes.resize(numPlanes);
	ReadPlanes(wwdFileStreamInflated);
	wwdFileStreamInflated.setIndex(tileDescriptionsOffset);
	ReadTileDescriptions(wwdFileStreamInflated);

	delete[] decompressedMainBlockVector;

	if (flags & WwdFlag_UseZCoords)
		sort(planes.begin(), planes.end(), [](const WwdPlane& a, const WwdPlane& b) { return a.coordZ < b.coordZ; });
}
void WapWwd::ReadPlanes(BufferReader& inputStream)
{
	uint32_t imageSetsOffset, objectsOffset;
	uint32_t imageSetsCount, objectsCount, tilesOffset;
	int64_t currIdx;

	for (WwdPlane& pln : planes)
	{
		inputStream.skip(8);
		inputStream.read(pln.flags);
		inputStream.skip(4);
		pln.name = inputStream.ReadString(64);
		inputStream.read(pln.pixelWidth);
		inputStream.read(pln.pixelHeight);
		inputStream.read(pln.tilePixelWidth);
		inputStream.read(pln.tilePixelHeight);
		inputStream.read(pln.tilesOnAxisX);
		inputStream.read(pln.tilesOnAxisY);
		inputStream.skip(8);
		pln.movementPercentX = inputStream.read<int32_t>() / 100.0f;
		pln.movementPercentY = inputStream.read<int32_t>() / 100.0f;
		inputStream.read(pln.fillColor);
		inputStream.read(imageSetsCount);
		inputStream.read(objectsCount);
		inputStream.read(tilesOffset);
		inputStream.read(imageSetsOffset);
		inputStream.read(objectsOffset);
		inputStream.read(pln.coordZ);

		pln.imageSets.resize(imageSetsCount);
		pln.objects.resize(objectsCount);

		currIdx = inputStream.getIndex() + 12; // save current index and skip junk values

		// Read plane tiles, image sets and objects
		inputStream.setIndex(tilesOffset);
		pln.ReadPlaneTiles(inputStream);
		inputStream.setIndex(imageSetsOffset);
		pln.ReadPlaneImageSets(inputStream);
		inputStream.setIndex(objectsOffset);
		pln.ReadPlaneObjects(inputStream);

		inputStream.setIndex(currIdx); // restore current index
	}
}
void WapWwd::ReadTileDescriptions(BufferReader& inputStream)
{
	uint32_t tileDescriptionsCount;

	inputStream.skip(8);
	inputStream.read(tileDescriptionsCount);
	inputStream.skip(20);

	tileDescriptions.resize(tileDescriptionsCount);

	for (WwdTileDescription& desc : tileDescriptions)
	{
		inputStream.read(desc.type);
		inputStream.skip(4);
		inputStream.read(desc.width);
		inputStream.read(desc.height);

		if (desc.type == WwdTileDescription::TileType_Single)
		{
			inputStream.read(desc.insideAttrib);
		}
		else
		{
			inputStream.read(desc.outsideAttrib);
			inputStream.read(desc.insideAttrib);
			ReadRect(inputStream, desc.rect);
		}
	}

	fixTilesDescription();
}
/*
 * Minor change to tiles:
 * In level 2 it's necessary to change the tiles for tower-cannons
   so the buullets will not collide with solid tiles.
 * In level 8 it's necessary to change the tiles for death blocks.
 * In levels 5, 9, and 11 it's necessary to change the tiles to
   cancel the groud-collision of the player with these tiles.
 * In level 14 it's necessary to change the tiles to cancel the
   groud-collision of projectiles with these tiles (and also to avoid
   invisible solid tiles).
 * In ActionPlane::addObject we set the BreakPlank rectangle to be
   the same as the tile rectangle.
 */
void WapWwd::fixTilesDescription()
{
	if (levelNumber == 2)
	{
		tileDescriptions[88].outsideAttrib = WwdTileDescription::TileAttribute_Clear;
		tileDescriptions[91].insideAttrib = WwdTileDescription::TileAttribute_Clear;
	}
	else if (levelNumber == 5)
	{
		tileDescriptions[509].insideAttrib = WwdTileDescription::TileAttribute_Clear;
	}
	else if (levelNumber == 8)
	{
		for (WwdPlane& pln : planes)
			if (pln.flags & WwdPlane::WwdPlaneFlags_MainPlane)
			{
				for (size_t i = 703; i < 720; pln.tiles[90][i++] = 184); // set death tiles behind Gabriel's ship
				break;
			}
	}
	else if (levelNumber == 9)
	{
		tileDescriptions[103].insideAttrib = WwdTileDescription::TileAttribute_Clear;
		tileDescriptions[104].insideAttrib = WwdTileDescription::TileAttribute_Clear;
		tileDescriptions[105].insideAttrib = WwdTileDescription::TileAttribute_Clear;
		tileDescriptions[106].insideAttrib = WwdTileDescription::TileAttribute_Clear;
		tileDescriptions[107].insideAttrib = WwdTileDescription::TileAttribute_Clear;
		tileDescriptions[108].insideAttrib = WwdTileDescription::TileAttribute_Clear;
		tileDescriptions[159].insideAttrib = WwdTileDescription::TileAttribute_Clear;
	}
	else if (levelNumber == 11)
	{
		tileDescriptions[39].insideAttrib = WwdTileDescription::TileAttribute_Clear;
	}
	else if (levelNumber == 14)
	{
		tileDescriptions[49].insideAttrib = WwdTileDescription::TileAttribute_Clear;
		tileDescriptions[50].insideAttrib = WwdTileDescription::TileAttribute_Clear;
		tileDescriptions[54].insideAttrib = WwdTileDescription::TileAttribute_Clear;
		tileDescriptions[62].insideAttrib = WwdTileDescription::TileAttribute_Clear;
		tileDescriptions[66].insideAttrib = WwdTileDescription::TileAttribute_Clear;
	}
}

WwdObject::WwdObject() :
	id(0), x(0), y(0), z(0), i(0), addFlags(0), dynamicFlags(0), drawFlags(0), userFlags(0),
	score(0), points(0), powerup(0), damage(0), smarts(0), health(0),
	moveRect({}), hitRect({}), attackRect({}), clipRect({}), userRect1({}), userRect2({}),
	userValue1(0), userValue2(0), userValue3(0), userValue4(0),
	userValue5(0), userValue6(0), userValue7(0), userValue8(0),
	minX(0), minY(0), maxX(0), maxY(0), speedX(0), speedY(0), tweakX(0), tweakY(0),
	counter(0), speed(0), width(0), height(0), direction(0), faceDir(0), timeDelay(0),
	frameDelay(0), objectType(0), hitTypeFlags(0), moveResX(0), moveResY(0)
{
}
