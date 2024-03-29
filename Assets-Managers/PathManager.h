#pragma once

#include <string>
#include <map>

using std::string;
using std::map;


class PathManager
{
public:
	static void setRoots(string prefix[4], string imageSet[4]);

	static void resetPaths();

	static string getImageSetPath(const string& imageSet);
	static string getAnimationSetPath(const string& aniSet);
	static string getAnimationPath(const string& aniPath);
	static string getSoundFilePath(const string& wavPath);
	static string getBackgroundMusicFilePath(const string& xmiPath);

private:
	static map<string, string> imageSetMap; // [prefix]=imageSet
	static map<string, string> data[5];
};
