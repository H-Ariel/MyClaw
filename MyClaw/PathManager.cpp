#include "framework.h"


string PathManager::_levelRoot;
map<string, string> PathManager::data[5];



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

void PathManager::resetPaths()
{
	for (auto& m : data)
		m.clear();
}

string PathManager::getImageSetPath(const string& _imageSet)
{
	if (data[0].count(_imageSet) == 0)
	{
		string imageSet(_imageSet);
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
		data[0][_imageSet] = imageSet;
	}

	return data[0][_imageSet];
}
string PathManager::getAnimationSetPath(const string& _aniSet)
{
	if (data[1].count(_aniSet) == 0)
	{
		string aniSet(_aniSet);
		aniSet = replaceString(aniSet, '_', '/');
		aniSet = replaceFirst(aniSet, "GAME", "GAME/ANIS");
		aniSet = replaceFirst(aniSet, "LEVEL", _levelRoot + "/ANIS");
		data[1][_aniSet] = aniSet;
	}

	return data[1][_aniSet];
}
string PathManager::getAnimationPath(const string& _path)
{
	if (data[2].count(_path) == 0)
	{
		string path(_path);
		path = getAnimationSetPath(path);
		path += ".ANI";
		// TODO: hack - something else
		if (path == "LEVEL1/ANIS/MANICALS/MANICAL.ANI" || path == "LEVEL1/ANIS/MANICALS/M.ANI")
		{
			path = "LEVEL1/ANIS/MANICLES/MANICAL.ANI";
		}

		data[2][_path] = path;
	}

	return data[2][_path];
}
string PathManager::getSoundFilePath(const string& _path)
{
	if (data[3].count(_path) == 0)
	{
		string path(_path);

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
		data[3][_path] = path + ".WAV";
	}

	return data[3][_path];
}
string PathManager::getBackgroundMusicFilePath(const string& _path)
{
	if (data[4].count(_path) == 0)
	{
		string path(_path);
		path = replaceString(path, '_', '/');
		path = replaceFirst(path, "GAME", "GAME/MUSIC");
		path = replaceFirst(path, "LEVEL", _levelRoot + "/MUSIC");
		data[4][_path] = path + ".XMI";
	}

	return data[4][_path];
}
