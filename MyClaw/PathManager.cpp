#include "PathManager.h"


string PathManager::_levelRoot;


string replaceFirst(string str, const string& src, const string& dst)
{
	if (contains(str, src))
		return str.replace(str.find(src), src.length(), dst);
	return str;
}


void PathManager::setLevelRoot(int lvlNo)
{
	_levelRoot = "LEVEL" + to_string(lvlNo);
}
string PathManager::getImageSetPath(string imageSet)
{
	if (!contains(imageSet, "IMAGES"))
	{
		imageSet = replaceFirst(imageSet, "GAME_", "GAME_IMAGES_");
		imageSet = replaceFirst(imageSet, "LEVEL_", "LEVEL_IMAGES_");
	}

	if (startsWith(imageSet, "LEVEL_"))
	{
		imageSet = replaceFirst(imageSet, "LEVEL", _levelRoot);
	}

	imageSet = replaceString(imageSet, '_', '/');

	return imageSet;
}
string PathManager::getAnimationSetPath(string aniSet)
{
	aniSet = replaceString(aniSet, '_', '/');
	aniSet = replaceFirst(aniSet, "GAME", "GAME/ANIS");
	aniSet = replaceFirst(aniSet, "LEVEL", _levelRoot + "/ANIS");
	return aniSet;
}
string PathManager::getAnimationPath(string path)
{
	path = getAnimationSetPath(path);
	path += ".ANI"; // TODO: move to caller ?

	// TODO: hack - something else
	if (path == "LEVEL1/ANIS/MANICALS/MANICAL.ANI" || path == "LEVEL1/ANIS/MANICALS/M.ANI")
	{
		path = "LEVEL1/ANIS/MANICLES/MANICAL.ANI";
	}

	return path;
}
string PathManager::getSoundFilePath(string path)
{
	if (_levelRoot == "LEVEL1" && path == "LEVEL_AMBIENT_ANGVIL")
	{
		path = "LEVEL_AMBIENT_ANVIL";
	}
	else if (_levelRoot == "LEVEL3")
	{
		if (path == "LEVEL_TRIGGER_1013")
		{
			path = "LEVEL_TRIGGER_1012";
		}
		else if (path == "LEVEL_TRIGGER_BIRDCALL2")
		{
			path = "LEVEL_AMBIENT_BIRDCALL2";
		}
	}

	path = replaceString(path, '_', '/');
	path = replaceFirst(path, "GAME", "GAME/SOUNDS");
	path = replaceFirst(path, "CLAW", "CLAW/SOUNDS");
	path = replaceFirst(path, "LEVEL", _levelRoot + "/SOUNDS");
	return path + ".WAV";
}
string PathManager::getBackgroundMusicFilePath(string path)
{
	path = replaceString(path, '_', '/');
	path = replaceFirst(path, "GAME", "GAME/MUSIC");
	path = replaceFirst(path, "LEVEL", _levelRoot + "/MUSIC");
	return path + ".XMI";
}
