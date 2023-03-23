#include "CrabNest.h"
#include "../AssetsManager.h"
#include "../ActionPlane.h"
#include "../Player.h"


CrabNest::CrabNest(const WwdObject& obj, Player* player)
	: BaseStaticPlaneObject(obj, player)
{
	_ani = AssetsManager::loadCopyAnimation("LEVEL7/ANIS/HATCHNEST.ANI", PathManager::getImageSetPath(obj.imageSet));
	_ani->position = position;
	_ani->updateFrames = false;
	_ani->loopAni = false;

	myMemCpy(_objRc, RectF((float)obj.minX, (float)obj.minY, (float)obj.maxX, (float)obj.maxY));

	WwdObject crabObj;
	crabObj.x = obj.x;
	crabObj.y = obj.y;
	crabObj.z = obj.z;
	crabObj.speedY = -600;
	crabObj.imageSet = "LEVEL_HERMITCRAB"; // TODO: add alse the black-crabs

	for (int32_t i = 0; i < obj.userValue1; i++)
	{
		crabObj.speedX = getRandomInt(-250, 250);
		_crabs.push_back(DBG_NEW HermitCrab(crabObj, _player, true));
	}
}
CrabNest::~CrabNest()
{
	for (HermitCrab* c : _crabs)
		delete c;
}

void CrabNest::Logic(uint32_t elapsedTime)
{
	if (_crabs.size() > 0 && CollisionDistances::isCollision(_player->GetRect(), _objRc))
	{
		_ani->updateFrames = true;

		for (HermitCrab* c : _crabs)
		{
			ActionPlane::addPlaneObject(c);
		}

		_crabs.clear();
	}

	_ani->Logic(elapsedTime);
}
