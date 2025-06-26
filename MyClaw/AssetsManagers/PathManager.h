#pragma once

#include "GameEngine/GameEngineFramework.hpp"


class PathManager
{
public:
	static void setRoots(string prefix[], string imageSet[]);

	static void resetPaths();

	static string getImageSetPath(const string& imageSet);
	static string getAnimationSetPath(const string& aniSet);
	static string getAnimationPath(const string& aniPath);
	static string getSoundFilePath(const string& wavPath);
	static string getBackgroundMusicFilePath(const string& xmiPath);

	static string getItemPath(int type, const string& imageSet);
	static int getItemType(const string& imageSet);

private:
	static  map<string, string>::iterator findImageSet(const string& imageSet);

	static map<string, string> imageSetMap;
	static map<string, string> data[5];
	static const map<string, int> ItemsMap;
	static map<string, string> ItemsPaths;
};
