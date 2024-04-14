#include "ImagesManager.h"
#include "GameEngine/WindowManager.h"


ImagesManager::ImagesManager(RezArchive* rezArchive) : _rezArchive(rezArchive) {}
ImagesManager::~ImagesManager()
{
	for (auto& i : _loadedBitmaps)
	{
		SafeRelease(i.second.first);
	}
}

shared_ptr<UIBaseImage> ImagesManager::loadImage(const string& path)
{
	shared_ptr<UIBaseImage> img;

	if (_loadedBitmaps.count(path))
	{
		img = allocNewSharedPtr<UIBaseImage>(_loadedBitmaps[path].first, _loadedBitmaps[path].second);
	}
	else
	{
		try
		{
			if (endsWith(path, ".PID"))
			{
				img = loadPidImage(path);
			}
			else if (endsWith(path, ".PCX"))
			{
				img = loadPcxImage(path);
			}
			else throw Exception("empty imgae");
		}
		catch (const Exception& ex)
		{
			img = allocNewSharedPtr<UIBaseImage>(nullptr); // empty image
			DBG_PRINT("WARNING: a blank image has been inserted. image path: \"%s\"\n", path.c_str());
		}

		_loadedBitmaps[path] = { img->_bitmap, img->offset };
	}

	return img;
}
void ImagesManager::clearLevelImages(const string& prefix)
{
	for (auto it = _loadedBitmaps.begin(); it != _loadedBitmaps.end();)
	{
		if (startsWith(it->first, prefix))
			it = _loadedBitmaps.erase(it);
		else
			++it;
	}
}

void fixPidOffset(string pidPath, int32_t& offsetX, int32_t& offsetY)
{
	// TODO: hack - edit the files?
	// NOTE: number-filenames are according to `fixFileName` at `RezArchive.cpp`


	// HUD pistol
	if (pidPath == "GAME/IMAGES/INTERFACE/WEAPONS/PISTOL/001.PID")
	{
		offsetY += 1;
	}
	// HUD magic
	else if (pidPath == "GAME/IMAGES/INTERFACE/WEAPONS/MAGIC/001.PID")
	{
		offsetY += 2;
	}
	else if (pidPath == "LEVEL7/IMAGES/SANDHOLE/005.PID")
	{
		offsetX -= 2;
	}
	// MagicClaw
	else if (pidPath == "CLAW/IMAGES/165.PID")
	{
		offsetX += 15;
	}
	else if (pidPath == "CLAW/IMAGES/166.PID")
	{
		offsetX += -5;
	}
	else if (pidPath == "CLAW/IMAGES/167.PID")
	{
		offsetX += 10;
	}
	// Claw swipe
	else if (pidPath == "CLAW/IMAGES/024.PID" || pidPath == "CLAW/IMAGES/025.PID" ||
		pidPath == "CLAW/IMAGES/026.PID" || pidPath == "CLAW/IMAGES/027.PID")
	{
		offsetX += 22;
	}
	// Cursor
	else if (pidPath == "GAME/IMAGES/CURSOR/004.PID")
	{
		offsetX += 2;
	}
	// Tower Cannon
	else if (pidPath == "LEVEL2/IMAGES/TOWERCANNONLEFT/002.PID" ||
		pidPath == "LEVEL2/IMAGES/TOWERCANNONLEFT/004.PID" ||
		pidPath == "LEVEL2/IMAGES/TOWERCANNONLEFT/005.PID")
	{
		offsetX -= 2;
	}
	else if (pidPath == "LEVEL2/IMAGES/TOWERCANNONRIGHT/002.PID"
		|| pidPath == "LEVEL2/IMAGES/TOWERCANNONRIGHT/004.PID"
		|| pidPath == "LEVEL2/IMAGES/TOWERCANNONRIGHT/005.PID")
	{
		offsetX += 2;
	}
	else if (pidPath == "LEVEL8/IMAGES/CANNONSWITCH/002.PID")
	{
		offsetX -= 2;
		offsetY -= 1;
	}
	// shooters from level 14
	else if (pidPath == "LEVEL14/IMAGES/SHOOTERS/LAUNCHLEFT/002.PID")
	{
		offsetX -= 2;
	}
	else if (pidPath == "LEVEL14/IMAGES/SHOOTERS/LAUNCHLEFT/003.PID")
	{
		offsetX -= 2;
	}
	else if (pidPath == "LEVEL14/IMAGES/SHOOTERS/LAUNCHLEFT/004.PID")
	{
		offsetX -= 2;
	}
	else if (pidPath == "LEVEL14/IMAGES/SHOOTERS/LAUNCHLEFT/005.PID")
	{
		offsetX -= 2;
	}
	else if (pidPath == "LEVEL14/IMAGES/SHOOTERS/LAUNCHLEFT/007.PID")
	{
		offsetX -= 2;
	}
	else if (pidPath == "LEVEL14/IMAGES/SHOOTERS/LAUNCHRIGHT/002.PID")
	{
		offsetX += 4;
	}
	else if (pidPath == "LEVEL14/IMAGES/SHOOTERS/LAUNCHRIGHT/003.PID")
	{
		offsetX += 2;
	}
	else if (pidPath == "LEVEL14/IMAGES/SHOOTERS/LAUNCHRIGHT/004.PID")
	{
		offsetX += 2;
	}
	else if (pidPath == "LEVEL14/IMAGES/SHOOTERS/LAUNCHRIGHT/005.PID")
	{
		offsetX += 2;
	}
	else if (pidPath == "LEVEL14/IMAGES/SHOOTERS/LAUNCHRIGHT/006.PID")
	{
		offsetX += 2;
	}
	else if (pidPath == "LEVEL14/IMAGES/SHOOTERS/LAUNCHRIGHT/007.PID")
	{
		offsetX += 5;
	}
	else if (pidPath == "LEVEL14/IMAGES/SHOOTERS/LAUNCHRIGHT/008.PID")
	{
		offsetX += 3;
	}
}
shared_ptr<UIBaseImage> ImagesManager::loadPidImage(const string& pidPath)
{
	WapPid pid(_rezArchive->getFile(pidPath)->getFileData(), _palette.get());

	fixPidOffset(pidPath, pid.offsetX, pid.offsetY);

	return allocNewSharedPtr<UIBaseImage>(
		WindowManager::createBitmapFromBuffer(pid.colors.data(), pid.width, pid.height),
		Point2F((float)pid.offsetX, (float)pid.offsetY));
}
shared_ptr<UIBaseImage> ImagesManager::loadPcxImage(const string& pcxPath)
{
	PcxFile pcx(_rezArchive->getFile(pcxPath)->getBufferReader());

	return allocNewSharedPtr<UIBaseImage>(
		WindowManager::createBitmapFromBuffer(pcx.colors.data(), pcx.width, pcx.height)
	);
}
