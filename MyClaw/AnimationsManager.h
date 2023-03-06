#pragma once

#include "RezArchive.h"
#include "Animation.h"

// TODO: make singleton ?

class AnimationsManager
{
public:
	AnimationsManager(RezArchive* rezArchive);

	shared_ptr<Animation> loadAnimation(const string& aniPath, const string& imageSetPath, bool save = true);
	shared_ptr<Animation> createAnimationFromDirectory(const string& dirPath, uint32_t duration, bool reversedOrder); // create new animation from directory content
	shared_ptr<Animation> createAnimationFromFromPidImage(const string& pidPath);

	void callAnimationsLogic(uint32_t elapsedTime);

	void clearLevelAnimations(const string& prefix);

private:
	map<string, shared_ptr<Animation>> _loadedAnimations;
	RezArchive* _rezArchive;
};
