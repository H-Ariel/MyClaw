#include "AnimationsManager.h"
#include "PathManager.h"


// return if we should update the enemy's idle animation
static inline bool isEnemyAniations(const string& dirPath)
{
	return (endsWith(dirPath, "/ANIS/OFFICER") ||
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
		endsWith(dirPath, "/ANIS/GABRIEL"));
}

AnimationsManager::AnimationsManager(RezArchive* rezArchive) : _rezArchive(rezArchive) {}

shared_ptr<Animation> AnimationsManager::loadAnimation(const string& aniPath, const string& imageSetPath, bool save)
{
	const string k = aniPath + '+' + imageSetPath;
	if (_loadedAnimations.count(k) == 0)
	{
		try
		{
			shared_ptr<Animation> ani(DBG_NEW Animation(getAnimationImages(aniPath, imageSetPath)));
			if (!save)
				return ani;
			_loadedAnimations[k] = ani;
		}
		catch (const Exception& ex)
		{
			throw Exception("Error while loading animation \"" + aniPath + "\". message: " + ex.message);
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
			for (auto& [name, file] : dir->_files)
			{
				path = file->getFullPath();
				if (endsWith(path, ".ANI"))
				{
					// get the animation's name (without the extension) and load it
					anis[name.substr(0, name.length() - 4)] = loadAnimation(path, imageSetPath, false);
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

	for (auto& [name, ani] : _savedAniDirs[dirPath])
		anis[name] = ani->getCopy();
	return anis;
}
shared_ptr<Animation> AnimationsManager::createAnimationFromDirectory(const string& dirPath, bool reversedOrder, uint32_t duration)
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

		for (auto& [name, file] : dir->_files)
		{
			if (file->isPidFile())
			{
				images.push_back(DBG_NEW Animation::FrameData(file->getFullPath(), duration));
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
	for (auto& [name, ani] : _loadedAnimations)
		ani->Logic(elapsedTime);
}
void AnimationsManager::clearLevelAnimations(const string& prefix)
{
	for (auto it = _loadedAnimations.begin(); it != _loadedAnimations.end();)
	{
		if (startsWith(it->first, prefix) )
			it = _loadedAnimations.erase(it);
		else
			++it;
	}

	for (auto it = _savedAniDirs.begin(); it != _savedAniDirs.end();)
	{
		if (startsWith(it->first, prefix) )
			it = _savedAniDirs.erase(it);
		else
			++it;
	}
}

vector<Animation::FrameData*> AnimationsManager::getAnimationImages(const string& aniPath, const string& _imageSetPath)
{
	vector<Animation::FrameData*> images;
	shared_ptr<BufferReader> aniFileReader = _rezArchive->getFileBufferReader(aniPath);
	string imageSetPath;
	string soundFilePath;
	char imgName[32];
	uint32_t imageSetPathLength, framesCount;
	uint16_t triggeredEventFlag, imageFileId, duration;
	bool useSoundFile;

	aniFileReader->skip(12);
	aniFileReader->read(framesCount);
	aniFileReader->read(imageSetPathLength);
	aniFileReader->skip(12);

	imageSetPath = aniFileReader->ReadString(imageSetPathLength);
	if (!_imageSetPath.empty())
		imageSetPath = _imageSetPath;
	imageSetPath = PathManager::getImageSetPath(imageSetPath);

	if (imageSetPath.empty())
	{
		throw Exception("empty imageSetPath at " + aniPath);
	}

	for (uint32_t i = 0; i < framesCount; i++, soundFilePath = "")
	{
		aniFileReader->read(triggeredEventFlag);
		useSoundFile = aniFileReader->read<uint8_t>();
		aniFileReader->skip(5);
		aniFileReader->read(imageFileId);
		aniFileReader->read(duration);
		aniFileReader->skip(8);

		if (triggeredEventFlag == 2)
		{
			soundFilePath = aniFileReader->ReadNullTerminatedString();

			if (!soundFilePath.empty() && useSoundFile)
			{
				if (endsWith(soundFilePath, "MLF") || endsWith(soundFilePath, "MRF"))
				{ // the rat use this sounds, and I hate it
					soundFilePath = "";
				}
				else
				{
					soundFilePath = PathManager::getSoundFilePath(soundFilePath);
				}
			}
		}

		// TODO: hack - something else
		if (startsWith(aniPath, "LEVEL2/ANIS/RAUX/BLOCK")) duration /= 2;
		else if (startsWith(aniPath, "LEVEL6/ANIS/WOLVINGTON/BLOCK")) duration /= 2;
		else if (aniPath == "LEVEL8/ANIS/GABRIELCANNON/HORZFIRE.ANI") duration *= 8;
		else if (aniPath == "LEVEL8/ANIS/GABRIELCANNON/VERTIFIRE.ANI") duration *= 8;
		else if (imageSetPath == "LEVEL8/IMAGES/GABRIELCANNON" && imageFileId == 0) imageFileId = 1;
		else if (aniPath == "LEVEL8/ANIS/CANNONSWITCH/SWITCH.ANI") duration *= 2;
		else if (aniPath == "LEVEL9/ANIS/SAWBLADE/SPIN.ANI") duration *= 4;
		else if (imageSetPath == "LEVEL10/IMAGES/MARROW" && imageFileId == 0) imageFileId = 1;
		else if (aniPath == "LEVEL14/ANIS/OMAR/HOME.ANI") duration = 250;

		sprintf(imgName, "/%03d.PID", imageFileId); // according to `fixFileName` at `RezArchive.cpp`

		images.push_back(DBG_NEW Animation::FrameData(imageSetPath + imgName, duration, soundFilePath));
	}

	return images;
}
