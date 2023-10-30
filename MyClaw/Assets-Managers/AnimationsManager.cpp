#include "AnimationsManager.h"


static inline bool isEnemyAniations(string dirPath)
{
	return (
		endsWith(dirPath, "/ANIS/OFFICER") ||
		endsWith(dirPath, "/ANIS/SOLDIER") ||
		endsWith(dirPath, "/ANIS/ROBBERTHIEF") ||
		endsWith(dirPath, "/ANIS/CUTTHROAT") ||
		endsWith(dirPath, "/ANIS/TOWNGUARD1") ||
		endsWith(dirPath, "/ANIS/TOWNGUARD2") ||
		endsWith(dirPath, "/ANIS/BEARSAILOR") ||
		endsWith(dirPath, "/ANIS/REDTAILPIRATE") ||
		endsWith(dirPath, "/ANIS/CRAZYHOOK") ||
		endsWith(dirPath, "/ANIS/PEGLEG") ||
		endsWith(dirPath, "/ANIS/MERCAT") ||
		endsWith(dirPath, "/ANIS/FISH") ||

		endsWith(dirPath, "/ANIS/GABRIEL")
		);

	// TODO: add all of them
}

AnimationsManager::AnimationsManager(RezArchive* rezArchive) : _rezArchive(rezArchive) {}

shared_ptr<Animation> AnimationsManager::loadAnimation(const string& aniPath, const string& imageSetPath, bool save)
{
	const string k = aniPath + '+' + imageSetPath;
	if (_loadedAnimations.count(k) == 0)
	{
		try
		{
			shared_ptr<Animation> ani(DBG_NEW Animation(_rezArchive, aniPath, imageSetPath));
			if (!save)
			{
				return ani;
			}
			_loadedAnimations[k] = ani;
		}
		catch (const Exception& ex)
		{
			throw Exception("Error while loading animation \"" + aniPath + "\". message: " + ex.what());
		}
	}
	return _loadedAnimations[k];
}
map<string, shared_ptr<Animation>> AnimationsManager::loadAnimationsFromDirectory(const string& dirPath, const string& imageSetPath)
{
	map<string, shared_ptr<Animation>> anis;

	if (_savedAniDirs.count(dirPath) == 0)
	{
		const RezDirectory* dir = _rezArchive->getDirectory(dirPath);
		if (dir)
		{
			string path;
			for (auto& i : dir->_files)
			{
				path = i.second->getFullPath();
				if (endsWith(path, ".ANI"))
				{
					// get the animation's name (without the extension) and load it
					anis[i.second->name.substr(0, i.second->name.length() - 4)] = loadAnimation(path, imageSetPath, false);
				}
			}
		}

		if (isEnemyAniations(dirPath))
		{
			vector<Animation::FrameData*> imagesList;

			for (auto i = anis.begin(); i != anis.end();)
			{
				if (startsWith(i->first, "STAND") || startsWith(i->first, "IDLE"))
				{
					for (Animation::FrameData* f : i->second->getImagesList())
					{
						if (f->duration == 0)
							myMemCpy(f->duration, 500U);
						imagesList.push_back(f);
					}
					i = anis.erase(i);
				}
				else
					i++;
			}

			anis["IDLE"] = allocNewSharedPtr<Animation>(imagesList);
		}
		else if (endsWith(dirPath, "/ANIS/SEAGULL"))
		{
			vector<Animation::FrameData*> imagesList;

			for (auto i = anis.begin(); i != anis.end();)
			{
				if (startsWith(i->first, "DIVE"))
				{
					for (Animation::FrameData* f : i->second->getImagesList())
					{
						if (f->duration == 0)
							myMemCpy(f->duration, 500U);
						imagesList.push_back(f);
					}
					i = anis.erase(i);
				}
				else
					i++;
			}

			anis["DIVE"] = allocNewSharedPtr<Animation>(imagesList);
		}

		_savedAniDirs[dirPath] = anis;
		anis.clear();
	}

	for (auto& i : _savedAniDirs[dirPath])
		anis[i.first] = i.second->getCopy();
	return anis;
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
			throw Exception("dir=null. path=" + dirPath);
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

	return _loadedAnimations[k];
}
shared_ptr<Animation> AnimationsManager::createAnimationFromPidImage(const string& pidPath)
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
	string prefix2 = '/' + prefix;
	for (auto it = _loadedAnimations.begin(); it != _loadedAnimations.end();)
	{
		if (startsWith(it->first, prefix) || startsWith(it->first, prefix2))
			it = _loadedAnimations.erase(it);
		else
			++it;
	}

	for (auto it = _savedAniDirs.begin(); it != _savedAniDirs.end();)
	{
		if (startsWith(it->first, prefix) || startsWith(it->first, prefix2))
			it = _savedAniDirs.erase(it);
		else
			++it;
	}
}
