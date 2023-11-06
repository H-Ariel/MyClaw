#include "DoNothing.h"
#include "../Assets-Managers/AssetsManager.h"


DoNothing::DoNothing(const WwdObject& obj)
	: BaseStaticPlaneObject(obj)
{
	string path(PathManager::getImageSetPath(obj.imageSet));

	if (obj.imageSet == "LEVEL_ARCHESFRONT" || obj.imageSet == "LEVEL_WANTED")
	{
		char str[9];
		int32_t i = obj.i;
		if (i == -1) i = 1;
		sprintf(str, "/%03d.PID", i);
		_ani = AssetsManager::createAnimationFromPidImage(path + str);
	}
	else if (obj.animation.empty())
	{
		if (contains(path, "SIGN/"))
			path[path.find_last_of('/')] = '_';

		_ani = AssetsManager::createAnimationFromDirectory(path);
	}
	else
	{
		_ani = AssetsManager::loadAnimation(PathManager::getAnimationPath(obj.animation), obj.imageSet);
	}

	/*
	// TODO: fix ZCoord
	if (startsWith(obj.logic, "Front")) myMemCpy<int32_t>(ZCoord, DefaultZCoord::FrontCrate);
	else if (startsWith(obj.logic, "Behind")) myMemCpy<int32_t>(ZCoord, DefaultZCoord::BackCrate);
	else if (obj.logic == "GooCoverup") myMemCpy<int32_t>(ZCoord, DefaultZCoord::Coverups);
	*/
}
