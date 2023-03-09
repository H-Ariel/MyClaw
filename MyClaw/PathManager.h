#pragma once

#include <string>

using std::string;


class PathManager
{
public:
	static void setLevelRoot(int lvlNo);
	static string getImageSetPath(string imageSet);
	static string getAnimationSetPath(string aniSet);
	static string getAnimationPath(string aniPath);
	static string getSoundFilePath(string wavPath);
	static string getBackgroundMusicFilePath(string xmiPath);

private:
	static string _levelRoot;
};
