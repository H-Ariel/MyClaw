#pragma once

#include "RezParser/libwap.h"
#include "GameEngine/UIAnimation.h"


class AnimationsManager
{
public:
	AnimationsManager(RezArchive* rezArchive);

	shared_ptr<UIAnimation> loadAnimation(const string& aniPath, const string& imageSetPath, bool save = true);
	map<string, shared_ptr<UIAnimation>> loadAnimationsFromDirectory(const string& dirPath, const string& imageSetPath); // load all ANI files in `dirPath`
	shared_ptr<UIAnimation> createAnimationFromDirectory(const string& dirPath, bool reversedOrder, uint32_t duration); // create new animation from directory content
	shared_ptr<UIAnimation> createAnimationFromPidImage(const string& pidPath);

	void callAnimationsLogic(uint32_t elapsedTime);

	void clearLevelAnimations(const string& prefix);

private:
	vector<UIAnimation::FrameData*> getAnimationImages(const string& aniPath, const string& _imageSetPath);

	map<string, shared_ptr<UIAnimation>> _loadedAnimations;
	map<string, map<string, shared_ptr<UIAnimation>>> _savedAniDirs;
	RezArchive* _rezArchive;
};
