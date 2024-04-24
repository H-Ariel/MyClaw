#pragma once

#include "Framework/BufferReader.h"


#define TILE_SIZE 64 // width and height of tile in pixels (by default)


struct WwdRect
{
	uint32_t left;
	uint32_t top;
	uint32_t right;
	uint32_t bottom;
};

struct WwdTileDescription
{
	enum WwdTileTypeFlags
	{
		TileType_Empty = 0,
		TileType_Single = 1,
		TileType_Double = 2,
	};
	enum WwdTileAttributeFlags
	{
		TileAttribute_Clear,
		TileAttribute_Solid = 1,
		TileAttribute_Ground = 2,
		TileAttribute_Climb = 3,
		TileAttribute_Death = 4,
	};

	uint32_t type;          // WwdTileTypeFlags
	uint32_t width;         // in pixels
	uint32_t height;        // in pixels
	uint32_t insideAttrib;  // WwdTileAttributeFlags
	// `outside_attrib` and `rect` only if `type == TileType_Double`
	uint32_t outsideAttrib; // WwdTileAttributeFlags
	WwdRect rect;
};

struct WwdObject
{
	enum AddFlags
	{
		AddFlag_Difficult = 1 << 0,
		AddFlag_EyeCandy = 1 << 1,
		AddFlag_HighDetail = 1 << 2,
		AddFlag_Multiplayer = 1 << 3,
		AddFlag_ExtraMemory = 1 << 4,
		AddFlag_FastCpu = 1 << 5
	};
	enum DynamicFlags
	{
		DynamicFlag_NoHit = 1 << 0,
		DynamicFlag_AlwaysActive = 1 << 1,
		DynamicFlag_Safe = 1 << 2,
		DynamicFlag_AutoHitDamage = 1 << 3
	};
	enum DrawFlags
	{
		NoDraw = 1 << 0,
		Mirror = 1 << 1,
		Invert = 1 << 2,
		Flash = 1 << 3,
	};
	enum UserFlags
	{
		UserFlag_1 = 1 << 0,
		UserFlag_2 = 1 << 1,
		UserFlag_3 = 1 << 2,
		UserFlag_4 = 1 << 3,
		UserFlag_5 = 1 << 4,
		UserFlag_6 = 1 << 5,
		UserFlag_7 = 1 << 6,
		UserFlag_8 = 1 << 7,
		UserFlag_9 = 1 << 8,
		UserFlag_10 = 1 << 9,
		UserFlag_11 = 1 << 10,
		UserFlag_12 = 1 << 11,
	};
	enum DirectionFlags
	{
		Direction_BottomLeft = 1,
		Direction_Down = 2,
		Direction_BottomRight = 3,
		Direction_Left = 4,
		Direction_NoMove = 5,
		Direction_Right = 6,
		Direction_TopLeft = 7,
		Direction_Up = 8,
		Direction_TopRight = 9
	};
	enum TypeFlags
	{
		Type_Generic = 1 << 0,
		Type_Player = 1 << 1,
		Type_Enemy = 1 << 2,
		Type_Powerup = 1 << 3,
		Type_Shot = 1 << 4,
		Type_Pshot = 1 << 5,
		Type_Eshot = 1 << 6,
		Type_Special = 1 << 7,
		Type_User1 = 1 << 8,
		Type_User2 = 1 << 9,
		Type_User3 = 1 << 10,
		Type_User4 = 1 << 11,
	};

	WwdObject();

	int32_t id; // Any value is accepted by WapWorld, but a good id should be positive and unique.
	int32_t x, y, z;
	int32_t i;
	uint32_t addFlags;     // AddFlags
	uint32_t dynamicFlags; // DynamicFlags
	uint32_t drawFlags;    // DrawFlags
	uint32_t userFlags;    // UserFlags
	int32_t score;
	int32_t points;
	int32_t powerup;
	int32_t damage;
	int32_t smarts;
	int32_t health;
	WwdRect moveRect, hitRect, attackRect, clipRect;
	WwdRect userRect1, userRect2;
	int32_t userValue1, userValue2, userValue3, userValue4;
	int32_t userValue5, userValue6, userValue7, userValue8;
	int32_t minX, minY;
	int32_t maxX, maxY;
	int32_t speedX, speedY;
	int32_t tweakX, tweakY;
	int32_t counter;
	int32_t speed;
	int32_t width, height;
	int32_t direction; // DirectionFlags
	int32_t faceDir;
	int32_t timeDelay;
	int32_t frameDelay;
	uint32_t objectType;   // TypeFlags single value
	uint32_t hitTypeFlags; // TypeFlags flags
	int32_t moveResX, moveResY;
	string name, logic, imageSet, animation;
};

class WwdPlane
{
public:
	enum WwdPlaneFlags
	{
		WwdPlaneFlags_MainPlane = 1 << 0,
		WwdPlaneFlags_NoDraw = 1 << 1,
		WwdPlaneFlags_XWrapping = 1 << 2,
		WwdPlaneFlags_YWrapping = 1 << 3,
		WwdPlaneFlags_AutoTileSize = 1 << 4
	};


	WwdPlane();


	uint32_t tilesWide, tilesHigh;
	uint32_t flags; // WwdPlaneFlags
	string name;
	uint32_t pixelWidth, pixelHeight;
	uint32_t tilePixelWidth, tilePixelHeight; // tile's size in pixels
	uint32_t tilesOnAxisX, tilesOnAxisY;
	float movementPercentX, movementPercentY;
	uint32_t fillColor;
	int32_t coordZ;
	vector<vector<int32_t>> tiles; // tiles[y][x] = id
	vector<string> imageSets;
	vector<WwdObject> objects;

private:
	void ReadPlaneTiles(BufferReader& inputStream);
	void ReadPlaneImageSets(BufferReader& inputStream);
	void ReadPlaneObjects(BufferReader& inputStream);

	friend class WapWwd;
};

class WapWwd // Wap World Data
{
public:
	WapWwd(shared_ptr<BufferReader> wwdReader, int levelNumber = 0);

	string levelName;
	string imageDirectoryPath;
	string rezPalettePath;
	string imageSet[4], prefix[4]; // imageSet and prefix are kind of dictionary: prefix[i] will be replaced by imageSet[i] in object's imageSet field
	vector<WwdPlane> planes;
	vector<WwdTileDescription> tileDescriptions;
	int32_t startX, startY;
	const int levelNumber; // not used in WapWwd, but useful when loading the game

private:
	void ReadPlanes(BufferReader& inputStream);
	void ReadTileDescriptions(BufferReader& inputStream);
	void fixTilesDescription();
};
