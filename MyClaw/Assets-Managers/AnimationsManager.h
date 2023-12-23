#pragma once

#include "Animation.h"


class AnimationsManager
{
public:
	AnimationsManager(RezArchive* rezArchive);

	shared_ptr<Animation> loadAnimation(const string& aniPath, const string& imageSetPath, bool save = true);
	map<string, shared_ptr<Animation>> loadAnimationsFromDirectory(const string& dirPath, const string& imageSetPath); // load all ANI files in `dirPath`
	shared_ptr<Animation> createAnimationFromDirectory(const string& dirPath, bool reversedOrder, uint32_t duration); // create new animation from directory content
	shared_ptr<Animation> createAnimationFromPidImage(const string& pidPath);

	void callAnimationsLogic(uint32_t elapsedTime);

	void clearLevelAnimations(const string& prefix);

private:
	map<string, shared_ptr<Animation>> _loadedAnimations;
	map<string, map<string, shared_ptr<Animation>>> _savedAniDirs;
	RezArchive* _rezArchive;
};
