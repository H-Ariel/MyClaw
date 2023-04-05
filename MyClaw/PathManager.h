#pragma once

#include <string>
#include <map>

using std::string;


class PathManager
{
public:
	static void setLevelRoot(int lvlNo);
	static void resetPaths();

	static string getImageSetPath(const string& imageSet);
	static string getAnimationSetPath(const string& aniSet);
	static string getAnimationPath(const string& aniPath);
	static string getSoundFilePath(const string& wavPath);
	static string getBackgroundMusicFilePath(const string& xmiPath);

private:
	static std::map<string, string> data[5];
	static string _levelRoot;
};
