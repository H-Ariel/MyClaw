#pragma once

#include "GameEngine/UIAnimation.h"
#include "RezParser/RezArchive.h"


class AnimationsManager
{
public:
	AnimationsManager(RezArchive* rezArchive);

	shared_ptr<UIAnimation> loadAnimation(const string& aniPath, const string& imageSetPath, bool save = true, const vector<ColorF>* colors = nullptr);
	unordered_map<string, shared_ptr<UIAnimation>> loadAnimationsFromDirectory(const string& dirPath, const string& imageSetPath, const vector<ColorF>* colors); // load all ANI files in `dirPath`
	shared_ptr<UIAnimation> createAnimationFromDirectory(const string& dirPath, bool reversedOrder, uint32_t duration); // create new animation from directory content
	shared_ptr<UIAnimation> createAnimationFromPidImage(const string& pidPath);

	void callAnimationsLogic(uint32_t elapsedTime);

	void clearAnimations(function <bool(const string&)> filter);

private:
	vector<UIAnimation::FrameData*> getAnimationImages(const string& aniPath, const string& defaultImageSetPath, const vector<ColorF>* colors = nullptr);
	
	unordered_map<string, shared_ptr<UIAnimation>> _loadedAnimations;
	unordered_map<string, unordered_map<string, shared_ptr<UIAnimation>>> _savedAniDirs;
	RezArchive* _rezArchive;
};
