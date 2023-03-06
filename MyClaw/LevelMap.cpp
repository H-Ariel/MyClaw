#include "LevelMap.h"
#include "ClawLevelEngine.h"
#include "AssetsManager.h"
#include "WindowManager.h"
#include "PathManager.h"


LevelMap::LevelMap(ClawLevelEngine* engine, uint8_t levelNumber)
	: _actionPlane(nullptr)
{
	_wwd = AssetsManager::loadWwdFile("LEVEL" + to_string((int)levelNumber) + "/WORLDS/WORLD.WWD");
	WindowManager::setTitle(_wwd->levelName);
	PathManager::setLevelRoot(levelNumber);

	engine->backgroundColor = _wwd->planes[0].fillColor;
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

Player* LevelMap::getPlayer()
{
	return _actionPlane->getPlayer();
}
const D2D1_POINT_2F* LevelMap::getWindowOffset()
{
	return &_actionPlane->position;
}
