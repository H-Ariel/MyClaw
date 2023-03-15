#include "LevelMap.h"
#include "AssetsManager.h"


LevelMap::LevelMap(int8_t levelNumber)
	: _actionPlane(nullptr)
{
	_wwd = AssetsManager::loadWwdFile("LEVEL" + to_string((int)levelNumber) + "/WORLDS/WORLD.WWD");
	PathManager::setLevelRoot(levelNumber);

	// TODO: move to `WapWorld()`
	if (levelNumber == 5)
	{
		_wwd->tilesDescription[509].insideAttrib = WwdTileDescription::TileAttribute_Clear;
		_wwd->tilesDescription[509].outsideAttrib = WwdTileDescription::TileAttribute_Clear;
	}

	for (WwdPlane& pln : _wwd->planes)
	{
		if (pln.isMainPlane)
		{
			_actionPlane = DBG_NEW ActionPlane(pln, _wwd);
			_planes.push_back(_actionPlane);
		}
		else
		{
#ifdef LOW_DETAILS
			if (pln.name != "Front")
#endif
			_planes.push_back(DBG_NEW LevelPlane(pln));
		}

		pln.objects.clear();
	}
}
LevelMap::~LevelMap()
{
	for (LevelPlane* p : _planes)
	{
		delete p;
	}
}

void LevelMap::Logic(uint32_t elapsedTime)
{
	_actionPlane->Logic(elapsedTime); // calls to player logic and set plane position
}
void LevelMap::Draw()
{
	for (LevelPlane* p : _planes)
	{
		p->position = _actionPlane->position;
		p->Draw();
	}
}
