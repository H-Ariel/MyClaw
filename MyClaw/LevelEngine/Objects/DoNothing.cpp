#include "DoNothing.h"


DoNothing::DoNothing(const WwdObject& obj)
	: BaseStaticPlaneObject(obj, false)
{
	string path(PathManager::getImageSetPath(obj.imageSet));

	if (obj.imageSet == "LEVEL_ARCHESFRONT" || obj.imageSet == "LEVEL_WANTED")
	{
		char str[9];
		int i = obj.i;
		if (i == -1) i = 1;
		sprintf(str, "/%03d.PID", i);
		_ani = AssetsManager::getAnimationFromPidImage(path + str);
	}
	else if (obj.animation.empty())
	{
		if (contains(path, "SIGN/"))
			path[path.find_last_of('/')] = '_';

		_ani = AssetsManager::getAnimationFromDirectory(path);
	}
	else
	{
		_ani = AssetsManager::loadAnimation(PathManager::getAnimationPath(obj.animation), obj.imageSet);
	}
}
