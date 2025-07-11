#include "AssetsManager.h"
#include "RezParser/AniFile.h"


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

static void fixAniDuration(const string& aniPath, const string& imageSetPath, int16_t imageFileId, uint16_t& duration)
{
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
}

AnimationsManager::AnimationsManager(RezArchive* rezArchive) : _rezArchive(rezArchive) {}

shared_ptr<UIAnimation> AnimationsManager::loadAnimation(const string& aniPath, const string& imageSetPath, bool save, const vector<ColorF>* colors)
{
	const string k = aniPath + '+' + imageSetPath;
	if (_loadedAnimations.count(k) == 0)
	{
		try
		{
			shared_ptr<UIAnimation> ani(DBG_NEW UIAnimation(getAnimationImages(aniPath, imageSetPath, colors)));
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
unordered_map<string, shared_ptr<UIAnimation>> AnimationsManager::loadAnimationsFromDirectory(const string& dirPath, const string& imageSetPath, const vector<ColorF>* colors)
{
	unordered_map<string, shared_ptr<UIAnimation>> anis;

	if (_savedAniDirs.count(dirPath) == 0)
	{
		const RezDirectory* dir = _rezArchive->getDirectory(dirPath);
		if (dir)
		{
			string path;
			for (auto& [name, file] : dir->rezFiles)
			{
				path = file->getFullPath();
				if (endsWith(path, ".ANI"))
				{
					// get the animation's name (without the extension) and load it
					anis[name.substr(0, name.length() - 4)] = loadAnimation(path, imageSetPath, false, colors);
				}
			}
		}

		if (isEnemyAniations(dirPath))
		{
			vector<UIAnimation::FrameData*> imagesList;

			for (auto i = anis.begin(); i != anis.end();)
			{
				if (startsWith(i->first, "STAND") || startsWith(i->first, "IDLE"))
				{
					for (UIAnimation::FrameData* f : i->second->getFramesList())
					{
						if (f->duration == 0)
							myMemCpy(f->duration, 500U);
						imagesList.push_back(f);
					}
					i = anis.erase(i);
				}
				else ++i;
			}

			anis["IDLE"] = make_shared<UIAnimation>(imagesList);
		}
		else if (endsWith(dirPath, "/ANIS/SEAGULL"))
		{
			vector<UIAnimation::FrameData*> imagesList;

			for (auto i = anis.begin(); i != anis.end();)
			{
				if (startsWith(i->first, "DIVE"))
				{
					for (UIAnimation::FrameData* f : i->second->getFramesList())
					{
						if (f->duration == 0)
							myMemCpy(f->duration, 500U);
						imagesList.push_back(f);
					}
					i = anis.erase(i);
				}
				else ++i;
			}

			anis["DIVE"] = make_shared<UIAnimation>(imagesList);
		}

		_savedAniDirs[dirPath] = anis;
		anis.clear();
	}

	for (auto& [name, ani] : _savedAniDirs[dirPath])
		anis[name] = ani->getCopy();
	return anis;
}
shared_ptr<UIAnimation> AnimationsManager::getAnimationFromDirectory(const string& dirPath, bool reversedOrder, uint32_t duration)
{
	const string k = dirPath + '+' + to_string(duration) + '+' + to_string(reversedOrder);
	if (_loadedAnimations.count(k) == 0)
	{
		vector<UIAnimation::FrameData*> images;

		const RezDirectory* dir = _rezArchive->getDirectory(dirPath);
		if (!dir || dir->rezFiles.size() == 0)
		{
			throw Exception("dir=null. path=" + dirPath);
		}

		for (auto& [name, file] : dir->rezFiles)
		{
			if (strcmp(file->extension, "PID") == 0)
			{
				images.push_back(DBG_NEW UIAnimation::FrameData(AssetsManager::loadImage(file->getFullPath()), duration));
			}
		}

		if (reversedOrder)
		{
			reverse(images.begin(), images.end());
		}

		_loadedAnimations[k] = make_shared<UIAnimation>(images);
	}

	return _loadedAnimations[k];
}
shared_ptr<UIAnimation> AnimationsManager::getAnimationFromPidImage(const string& pidPath)
{
	if (_loadedAnimations.count(pidPath) == 0)
	{
		vector<UIAnimation::FrameData*> images = { DBG_NEW UIAnimation::FrameData(AssetsManager::loadImage(pidPath)) };
		_loadedAnimations[pidPath] = make_shared<UIAnimation>(images);
		_loadedAnimations[pidPath]->updateFrames = false;
	}
	return _loadedAnimations[pidPath];
}

void AnimationsManager::callAnimationsLogic(uint32_t elapsedTime)
{
	for (auto& [name, ani] : _loadedAnimations)
		ani->Logic(elapsedTime);
}
void AnimationsManager::clearAnimations(function <bool(const string&)> filter)
{
	for (auto i = _loadedAnimations.begin(); i != _loadedAnimations.end();)
	{
		if (filter(i->first))
			i = _loadedAnimations.erase(i);
		else
			++i;
	}

	for (auto i = _savedAniDirs.begin(); i != _savedAniDirs.end();)
	{
		if (filter(i->first))
			i = _savedAniDirs.erase(i);
		else
			++i;
	}
}

vector<UIAnimation::FrameData*> AnimationsManager::getAnimationImages(const string& aniPath, const string& defaultImageSetPath, const vector<ColorF>* colors)
{
	WapAni ani(_rezArchive->getFile(aniPath)->getFileReader());

	vector<UIAnimation::FrameData*> images;
	string imageSetPath;
	char imgName[32];

	imageSetPath = ani.imageSetPath;
	if (!defaultImageSetPath.empty())
		imageSetPath = defaultImageSetPath;
	imageSetPath = PathManager::getImageSetPath(imageSetPath);

	if (imageSetPath.empty())
	{
		throw Exception("empty imageSetPath at " + aniPath);
	}

	for (WapAni::Frame frame : ani.animationFrames)
	{
		if (!frame.eventFilePath.empty() && frame.useSoundFile)
		{
			if (endsWith(frame.eventFilePath, "MLF") || endsWith(frame.eventFilePath, "MRF"))
			{ // the rat use this sounds, and I hate it
				frame.eventFilePath = "";
			}
			else
			{
				frame.eventFilePath = PathManager::getSoundFilePath(frame.eventFilePath);
			}
		}

		fixAniDuration(aniPath, imageSetPath, frame.imageFileId, frame.duration);

		sprintf(imgName, "/%03d.PID", frame.imageFileId); // according to `fixFileName` at `RezArchive.cpp`

		images.push_back(DBG_NEW UIAnimation::FrameData(
			AssetsManager::loadImage(imageSetPath + imgName, colors),
			frame.duration, frame.eventFilePath)
		);
	}

	return images;
}
