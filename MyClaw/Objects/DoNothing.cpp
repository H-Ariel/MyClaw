#include "DoNothing.h"
#include "../AssetsManager.h"


DoNothing::DoNothing(const WwdObject& obj)
	: BaseStaticPlaneObject(obj)
{
	string path(PathManager::getImageSetPath(obj.imageSet));

	if (obj.imageSet == "LEVEL_ARCHESFRONT" || obj.imageSet == "LEVEL_WANTED") // TODO: somthing else
	{
		char str[9];
		int32_t i = obj.i;
		if (i == -1) i = 1;
		sprintf(str, "/%03d.PID", i);
		_ani = AssetsManager::createAnimationFromFromPidImage(path + str);
	}
	else if (obj.animation.empty())
	{
		if (contains(path, "SIGN/"))
			path[path.find_last_of('/')] = '_';

		_ani = AssetsManager::createAnimationFromDirectory(path, 125, false);
	}
	else
	{
		_ani = AssetsManager::loadAnimation(PathManager::getAnimationPath(obj.animation), obj.imageSet);
	}
}
