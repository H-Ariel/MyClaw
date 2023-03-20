#include "AnimationsManager.h"


AnimationsManager::AnimationsManager(RezArchive* rezArchive)
	: _rezArchive(rezArchive)
{
}

shared_ptr<Animation> AnimationsManager::loadAnimation(const string& aniPath, const string& imageSetPath, bool save)
{
	const string k = aniPath + '+' + imageSetPath;
	if (_loadedAnimations.count(k) == 0)
	{
		try 
		{
			shared_ptr<Animation> ani = allocNewSharedPtr<Animation>(_rezArchive, aniPath, imageSetPath);
			if (!save)
			{
				return ani;
			}
			_loadedAnimations[k] = ani;
		}
		catch (Exception ex)
		{
			throw Exception(__FUNCTION__ ": " + ex.what() + '\n');
		}
	}
	return _loadedAnimations[k];
}
shared_ptr<Animation> AnimationsManager::createAnimationFromDirectory(const string& dirPath, uint32_t duration, bool reversedOrder)
{
	const string k = dirPath + '+' + to_string(duration) + '+' + to_string(reversedOrder);
	if (_loadedAnimations.count(k) == 0)
	{
		vector<Animation::FrameData*> images;

		const RezDirectory* dir = _rezArchive->getDirectory(dirPath);
		if (!dir || dir->_files.size() == 0)
		{
			throw Exception(__FUNCTION__": dir=null. path=" + dirPath);
		}

		for (auto& i : dir->_files)
		{
			if (i.second->isPidFile())
			{
				images.push_back(DBG_NEW Animation::FrameData(i.second->getFullPath(), duration));
			}
		}

		if (reversedOrder)
		{
			reverse(images.begin(), images.end());
		}

		_loadedAnimations[k] = allocNewSharedPtr<Animation>(images);
	}

	// TODO: if ani.images.size=0: ani.update=false

	return _loadedAnimations[k];
}
shared_ptr<Animation> AnimationsManager::createAnimationFromFromPidImage(const string& pidPath)
{
	if (_loadedAnimations.count(pidPath) == 0)
	{
		vector<Animation::FrameData*> images = { DBG_NEW Animation::FrameData(pidPath) };
		_loadedAnimations[pidPath] = allocNewSharedPtr<Animation>(images);
		_loadedAnimations[pidPath]->updateFrames = false;
	}
	return _loadedAnimations[pidPath];
}

void AnimationsManager::callAnimationsLogic(uint32_t elapsedTime)
{
	for (auto& i : _loadedAnimations)
	{
		i.second->Logic(elapsedTime);
	}
}

void AnimationsManager::clearLevelAnimations(const string& prefix)
{
	vector<string> keysToRemove;
	for (auto& a : _loadedAnimations)
	{
		if (startsWith(a.first, prefix))
		{
			keysToRemove.push_back(a.first);
		}
	}

	for (const string& k : keysToRemove)
	{
		_loadedAnimations.erase(k);
	}
}
