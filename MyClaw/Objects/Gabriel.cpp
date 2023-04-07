#include "Gabriel.h"
#include "../Player.h"
#include "../AssetsManager.h"


// TODO: continue write Gabriel class


Gabriel::Gabriel(const WwdObject& obj, Player* player)
	: BaseBoss(obj, player, 10, "", "HITHIGH", "HITLOW", "KILLFALL4", "", "", "")
{
	_forward = false;

	_ani = _animations["IDLE2"];
	_health = 2;
}

void Gabriel::Logic(uint32_t elapsedTime)
{
	PreLogic(elapsedTime);
	PostLogic(elapsedTime);
}

pair<Rectangle2D, int8_t> Gabriel::GetAttackRect()
{
	return {};
}

void Gabriel::makeAttack()
{
}

bool Gabriel::checkForHurts()
{
	return BaseEnemy::checkForHurts();
}




GabrielCannon::GabrielCannon(const WwdObject& obj, Player* player)
	: BaseStaticPlaneObject(obj, player)
{
	string aniSet = PathManager::getAnimationSetPath(obj.imageSet);
	string imageSet = PathManager::getImageSetPath(obj.imageSet);

	_home = AssetsManager::loadAnimation(aniSet + "/HOME.ANI", imageSet); // nop
	_rest = AssetsManager::loadAnimation(aniSet + "/REST.ANI", imageSet); // go down
	_rise = AssetsManager::loadAnimation(aniSet + "/RISE.ANI", imageSet); // go up
	_horzfire = AssetsManager::loadAnimation(aniSet + "/HORZFIRE.ANI", imageSet); // horizontal fire
	_vertfire = AssetsManager::loadAnimation(aniSet + "/VERTIFIRE.ANI", imageSet); // vertical fire

	_ani = _home;
}

void GabrielCannon::Logic(uint32_t elapsedTime)
{
	_ani->Logic(elapsedTime);
}
