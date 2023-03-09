#include "DoNothing.h"
#include "../AssetsManager.h"


DoNothing::DoNothing(const WwdObject& obj)
	: BaseStaticPlaneObject(obj)
{
	string path(PathManager::getImageSetPath(obj.imageSet));

	if (obj.imageSet == "LEVEL_ARCHESFRONT")
	{
		_ani = AssetsManager::createAnimationFromFromPidImage(path + getArchesfrontImagePath(obj.i));
	}
	else
	{
		if (obj.animation.empty())
		{
			// TODO: something else
			if (contains(path, "ARROWSIGN/"))
				path[path.find_last_of('/')] = '_';

			_ani = AssetsManager::createAnimationFromDirectory(path);
		}
		else
		{
			_ani = AssetsManager::loadAnimation(PathManager::getAnimationPath(obj.animation), obj.imageSet);
		}
	}
}
string DoNothing::getArchesfrontImagePath(int32_t objI)
{
	char str[9];
	if (objI < 1 || 5 < objI)
		objI = 1;
	sprintf(str, "/%03d.PID", objI);
	return str;
}
