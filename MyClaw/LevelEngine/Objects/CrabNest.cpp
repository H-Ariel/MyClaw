#include "CrabNest.h"
#include "../GlobalObjects.h"


CrabNest::CrabNest(const WwdObject& obj)
	: BaseStaticPlaneObject(obj)
{
	_ani = AssetsManager::loadCopyAnimation("LEVEL7/ANIS/HATCHNEST.ANI", PathManager::getImageSetPath(obj.imageSet));
	_ani->position = position;
	_ani->updateFrames = false;
	_ani->loopAni = false;

	myMemCpy(_objRc, Rectangle2D((float)obj.minX, (float)obj.minY, (float)obj.maxX, (float)obj.maxY));

	WwdObject crabObj;
	crabObj.x = obj.x;
	crabObj.y = obj.y;
	crabObj.z = obj.z;
	crabObj.speedY = -600;
	crabObj.imageSet = "LEVEL_HERMITCRAB"; // TODO: add also the black-crabs

	int32_t treasuresList[9] = {};
	treasuresList[0] = obj.powerup;
	memcpy(treasuresList + 1, &obj.userRect1, sizeof(int32_t) * 4);
	memcpy(treasuresList + 5, &obj.userRect2, sizeof(int32_t) * 4);

	// TODO: make sure we used all the treasures
	for (int i = 0; i < obj.userValue1; i++)
	{
		crabObj.powerup = treasuresList[i];
		crabObj.speedX = getRandomInt(-250, 250);
		_crabs.push_back(DBG_NEW HermitCrab(crabObj, true));
	}
}
CrabNest::~CrabNest()
{
	for (HermitCrab* c : _crabs)
		delete c;
}

void CrabNest::Logic(uint32_t elapsedTime)
{
	if (_crabs.size() > 0 && GO::getPlayerRect().intersects(_objRc))
	{
		_ani->updateFrames = true;

		for (HermitCrab* c : _crabs)
			GO::addObjectToActionPlane(c);

		_crabs.clear();
	}

	_ani->Logic(elapsedTime);
}
