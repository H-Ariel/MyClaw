#include "PathManager.h"
#include "Framework/framework.h"


enum Types
{
	IMAGE_SET,
	ANIMATION_SET,
	ANIMATION,
	SOUND,
	BACKGROUND_MUSIC
};

map<string, string> PathManager::data[5]; // [type][path]=result
map<string, string> PathManager::imageSetMap;


string replaceFirst(string str, const string& src, const string& dst)
{
	if (contains(str, src))
		return str.replace(str.find(src), src.length(), dst);
	return str;
}

void PathManager::setRoots(string prefix[], string imageSet[])
{
	for (int8_t i = 0; i < 4; i++)
		if (!prefix[i].empty())
			imageSetMap[prefix[i]] = fixPath(imageSet[i]);
}

void PathManager::resetPaths()
{
	for (auto& m : data)
		m.clear();
	imageSetMap.clear();
}

string PathManager::getImageSetPath(const string& _imageSet)
{
	if (data[IMAGE_SET].count(_imageSet) == 0)
	{
		string newImageSet(_imageSet);

		if (!contains(newImageSet, "IMAGES"))
			for (const auto& [prefix, imageSet] : imageSetMap)
				if (startsWith(newImageSet, prefix + '_'))
				{
					newImageSet = replaceFirst(newImageSet, prefix, imageSet);
					break;
				}

		newImageSet = replaceString(newImageSet, '_', '/');
		data[IMAGE_SET][_imageSet] = newImageSet;
	}

	return data[IMAGE_SET][_imageSet];
}
string PathManager::getAnimationSetPath(const string& _aniSet)
{
	if (data[ANIMATION_SET].count(_aniSet) == 0)
	{
		string aniSet(_aniSet);

		for (const auto& [prefix, imageSet] : imageSetMap)
		{
			if (startsWith(aniSet, prefix + '_'))
			{
				aniSet = replaceFirst(aniSet, prefix, replaceFirst(imageSet, "IMAGES", "ANIS"));
				break;
			}
		}

		aniSet = replaceString(aniSet, '_', '/');
		if (aniSet == "LEVEL12/ANIS/KINGAQUATIS")
			aniSet = "LEVEL12/ANIS/AQUATIS";
		data[ANIMATION_SET][_aniSet] = aniSet;
	}

	return data[ANIMATION_SET][_aniSet];
}
string PathManager::getAnimationPath(const string& _path)
{
	if (data[ANIMATION].count(_path) == 0)
	{
		string path(_path);
		path = getAnimationSetPath(path);
		path += ".ANI";
		// TODO: hack - something else
		if (path == "LEVEL1/ANIS/MANICALS/MANICAL.ANI" || path == "LEVEL1/ANIS/MANICALS/M.ANI")
		{
			path = "LEVEL1/ANIS/MANICLES/MANICAL.ANI";
		}

		data[ANIMATION][_path] = path;
	}

	return data[ANIMATION][_path];
}
string PathManager::getSoundFilePath(const string& _path)
{
	if (data[SOUND].count(_path) == 0)
	{
		string path(_path);

		// TODO: hack - something else
		if (imageSetMap["LEVEL"] == "LEVEL1" && path == "LEVEL_AMBIENT_ANGVIL")
		{
			path = "LEVEL_AMBIENT_ANVIL";
		}
		else if (imageSetMap["LEVEL"] == "LEVEL3_IMAGES")
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

		path = replaceFirst(path, "CLAW", "CLAW/SOUNDS");

		for (const auto& [prefix, imageSet] : imageSetMap)
			if (startsWith(path, prefix + '_'))
			{
				path = replaceFirst(path, prefix, replaceFirst(imageSet, "IMAGES", "SOUNDS"));
				break;
			}

		path = replaceString(path, '_', '/');
		data[SOUND][_path] = path + ".WAV";
	}

	return data[SOUND][_path];
}
string PathManager::getBackgroundMusicFilePath(const string& _path)
{
	if (data[BACKGROUND_MUSIC].count(_path) == 0)
	{
		string path(_path);

		for (const auto& [prefix, imageSet] : imageSetMap)
			if (startsWith(path, prefix + '_'))
			{
				path = replaceFirst(path, prefix, replaceFirst(imageSet, "IMAGES", "MUSIC"));
				break;
			}

		path = replaceString(path, '_', '/');
		data[BACKGROUND_MUSIC][_path] = path + ".XMI";
	}

	return data[BACKGROUND_MUSIC][_path];
}
