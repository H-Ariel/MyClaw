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

		_ani = AssetsManager::createAnimationFromDirectory(path, 100, false);
	}
	else
	{
		_ani = AssetsManager::loadAnimation(PathManager::getAnimationPath(obj.animation), obj.imageSet);
	}

	// TODO: find perfect ZCoord for some of the objects (e.g. at level 11)
}
