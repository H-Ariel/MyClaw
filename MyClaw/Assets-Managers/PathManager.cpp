#include "PathManager.h"
#include "Framework/Framework.h"
#include "../Objects/Item.h"


enum Types
{
	IMAGE_SET,
	ANIMATION_SET,
	ANIMATION,
	SOUND,
	BACKGROUND_MUSIC
};

map<string, string> PathManager::imageSetMap; // [prefix]=imageSet
map<string, string> PathManager::data[5]; // [type][path]=result

const map<string, int> PathManager::ItemsMap = {
	{ "GAME_TREASURE_GOLDBARS", Item::Treasure_Goldbars },
	{ "GAME_TREASURE_RINGS_RED", Item::Treasure_Rings_Red },
	{ "GAME_TREASURE_RINGS_GREEN", Item::Treasure_Rings_Green },
	{ "GAME_TREASURE_RINGS_BLUE", Item::Treasure_Rings_Blue },
	{ "GAME_TREASURE_RINGS_PURPLE", Item::Treasure_Rings_Purple },
	{ "GAME_TREASURE_NECKLACE", Item::Treasure_Necklace },
	{ "GAME_TREASURE_CROSSES_RED", Item::Treasure_Crosses_Red },
	{ "GAME_TREASURE_CROSSES_GREEN", Item::Treasure_Crosses_Green },
	{ "GAME_TREASURE_CROSSES_BLUE", Item::Treasure_Crosses_Blue },
	{ "GAME_TREASURE_CROSSES_PURPLE", Item::Treasure_Crosses_Purple },
	{ "GAME_TREASURE_SCEPTERS_RED", Item::Treasure_Scepters_Red },
	{ "GAME_TREASURE_SCEPTERS_GREEN", Item::Treasure_Scepters_Green },
	{ "GAME_TREASURE_SCEPTERS_BLUE", Item::Treasure_Scepters_Blue },
	{ "GAME_TREASURE_SCEPTERS_PURPLE", Item::Treasure_Scepters_Purple },
	{ "GAME_TREASURE_GECKOS_RED", Item::Treasure_Geckos_Red },
	{ "GAME_TREASURE_GECKOS_GREEN", Item::Treasure_Geckos_Green },
	{ "GAME_TREASURE_GECKOS_BLUE", Item::Treasure_Geckos_Blue },
	{ "GAME_TREASURE_GECKOS_PURPLE", Item::Treasure_Geckos_Purple },
	{ "GAME_AMMO_DEATHBAG", Item::Ammo_Deathbag },
	{ "GAME_AMMO_SHOT", Item::Ammo_Shot },
	{ "GAME_AMMO_SHOTBAG", Item::Ammo_Shotbag },
	{ "GAME_CATNIPS_NIP1", Item::Powerup_Catnip_White },
	{ "GAME_CATNIPS_NIP2", Item::Powerup_Catnip_Red },
	{ "LEVEL_HEALTH", Item::Health_Level },
	{ "GAME_HEALTH_BREADWATER", Item::Health_Level },
	{ "GAME_HEALTH_POTION3", Item::Health_25 },
	{ "GAME_HEALTH_POTION1", Item::Health_10 },
	{ "GAME_HEALTH_POTION2", Item::Health_15 },
	{ "GAME_MAGIC_GLOW", Item::Ammo_Magic_5 },
	{ "GAME_MAGIC_STARGLOW", Item::Ammo_Magic_10 },
	{ "GAME_MAGIC_MAGICCLAW", Item::Ammo_Magic_25 },
	{ "GAME_MAPPIECE", Item::MapPiece },
	{ "GAME_WARP", Item::Warp },
	{ "GAME_VERTWARP", Item::Warp },
	{ "GAME_TREASURE_COINS", Item::Treasure_Coins },
	{ "GAME_DYNAMITE", Item::Ammo_Dynamite },
	{ "GAME_CURSES_AMMO", Item::Curse_Ammo },
	{ "GAME_CURSES_MAGIC", Item::Curse_Magic },
	{ "GAME_CURSES_HEALTH", Item::Curse_Health },
	{ "GAME_CURSES_LIFE", Item::Curse_Life },
	{ "GAME_CURSES_TREASURE", Item::Curse_Treasure },
	{ "GAME_CURSES_FREEZE", Item::Curse_Freeze },
	{ "GAME_TREASURE_CHALICES_RED", Item::Treasure_Chalices_Red },
	{ "GAME_TREASURE_CHALICES_GREEN", Item::Treasure_Chalices_Green },
	{ "GAME_TREASURE_CHALICES_BLUE", Item::Treasure_Chalices_Blue },
	{ "GAME_TREASURE_CHALICES_PURPLE", Item::Treasure_Chalices_Purple },
	{ "GAME_TREASURE_CROWNS_RED", Item::Treasure_Crowns_Red },
	{ "GAME_TREASURE_CROWNS_GREEN", Item::Treasure_Crowns_Green },
	{ "GAME_TREASURE_CROWNS_BLUE", Item::Treasure_Crowns_Blue },
	{ "GAME_TREASURE_CROWNS_PURPLE", Item::Treasure_Crowns_Purple },
	{ "GAME_TREASURE_JEWELEDSKULL_RED", Item::Treasure_Skull_Red },
	{ "GAME_TREASURE_JEWELEDSKULL_GREEN", Item::Treasure_Skull_Green },
	{ "GAME_TREASURE_JEWELEDSKULL_BLUE", Item::Treasure_Skull_Blue },
	{ "GAME_TREASURE_JEWELEDSKULL_PURPLE", Item::Treasure_Skull_Purple },
	{ "GAME_POWERUPS_GHOST", Item::Powerup_Invisibility },
	{ "GAME_POWERUPS_INVULNERABLE", Item::Powerup_Invincibility },
	{ "GAME_POWERUPS_EXTRALIFE", Item::Powerup_ExtraLife },
	{ "GAME_POWERUPS_LIGHTNINGSWORD", Item::Powerup_LightningSword },
	{ "GAME_POWERUPS_FIRESWORD", Item::Powerup_FireSword },
	{ "GAME_POWERUPS_ICESWORD", Item::Powerup_IceSword },
	{ "GAME_BOSSWARP", Item::BossWarp },
	{ "LEVEL_GEM", Item::NineLivesGem }
};
map<string, string> PathManager::ItemsPaths;


static string replaceFirst(string str, const string& src, const string& dst)
{
	if (contains(str, src))
		return str.replace(str.find(src), src.length(), dst);
	return str;
}
// replace underscores with slashes
static string updatePath(string path)
{
	replace(path.begin(), path.end(), '_', '/');
	return path;
}


void PathManager::setRoots(string prefix[], string imageSet[])
{
	for (int8_t i = 0; i < 4; i++)
		if (!prefix[i].empty())
			imageSetMap[prefix[i]] = fixPath(imageSet[i]);
}

void PathManager::resetPaths()
{
	for (auto& m : data)
		m.clear();
	imageSetMap.clear();

	ItemsPaths.erase("LEVEL_GEM");
	ItemsPaths.erase("LEVEL_HEALTH");
	ItemsPaths.erase("GAME_HEALTH_BREADWATER");
}

string PathManager::getImageSetPath(const string& imageSet)
{
	if (data[IMAGE_SET].count(imageSet) == 0)
	{
		string newImageSet;

		if (!contains(imageSet, "IMAGES"))
		{
			auto it = findImageSet(imageSet);
			if (it != imageSetMap.end())
				newImageSet = replaceFirst(imageSet, it->first, it->second);
		}

		if (newImageSet.empty())
			newImageSet = imageSet;

		data[IMAGE_SET][imageSet] = updatePath(newImageSet);
	}

	return data[IMAGE_SET][imageSet];
}
string PathManager::getAnimationSetPath(const string& aniSet)
{
	if (data[ANIMATION_SET].count(aniSet) == 0)
	{
		string newAniSet;

		auto it = findImageSet(aniSet);
		if (it != imageSetMap.end())
			newAniSet = replaceFirst(aniSet, it->first, replaceFirst(it->second, "IMAGES", "ANIS"));

		if (newAniSet.empty())
			newAniSet = aniSet;

		newAniSet = updatePath(newAniSet);
		if (newAniSet == "LEVEL12/ANIS/KINGAQUATIS")
			newAniSet = "LEVEL12/ANIS/AQUATIS";
		data[ANIMATION_SET][aniSet] = newAniSet;
	}

	return data[ANIMATION_SET][aniSet];
}
string PathManager::getAnimationPath(const string& path)
{
	if (data[ANIMATION].count(path) == 0)
	{
		string newPath = getAnimationSetPath(path);
		// TODO: hack - something else
		if (newPath == "LEVEL1/ANIS/MANICALS/MANICAL" || newPath == "LEVEL1/ANIS/MANICALS/M")
			newPath = "LEVEL1/ANIS/MANICLES/MANICAL";

		data[ANIMATION][path] = newPath + ".ANI";
	}

	return data[ANIMATION][path];
}
string PathManager::getSoundFilePath(const string& path)
{
	if (data[SOUND].count(path) == 0)
	{
		string newPath;

		// TODO: hack - something else
		if (imageSetMap["LEVEL"] == "LEVEL1" && path == "LEVEL_AMBIENT_ANGVIL")
		{
			newPath = "LEVEL_AMBIENT_ANVIL";
		}
		else if (imageSetMap["LEVEL"] == "LEVEL3_IMAGES")
		{
			if (path == "LEVEL_TRIGGER_1013")
				newPath = "LEVEL_TRIGGER_1012";
			else if (path == "LEVEL_TRIGGER_BIRDCALL2")
				newPath = "LEVEL_AMBIENT_BIRDCALL2";
		}

		if (newPath.empty())
			newPath = path;

		newPath = replaceFirst(newPath, "CLAW", "CLAW/SOUNDS");

		auto it = findImageSet(newPath);
		if (it != imageSetMap.end())
			newPath = replaceFirst(newPath, it->first, replaceFirst(it->second, "IMAGES", "SOUNDS"));

		data[SOUND][path] = updatePath(newPath) + ".WAV";
	}

	return data[SOUND][path];
}
string PathManager::getBackgroundMusicFilePath(const string& path)
{
	if (data[BACKGROUND_MUSIC].count(path) == 0)
	{
		string newPath;

		auto it = findImageSet(path);
		if (it == imageSetMap.end())
			newPath = path;
		else
			newPath = replaceFirst(path, it->first, replaceFirst(it->second, "IMAGES", "MUSIC"));

		data[BACKGROUND_MUSIC][path] = updatePath(newPath) + ".XMI";
	}

	return data[BACKGROUND_MUSIC][path];
}

string PathManager::getItemPath(int type, const string& imageSet)
{
	if (type == Item::Warp)
	{
		// because we have WARP and VERTWARP
		return PathManager::getImageSetPath(imageSet);
	}

	for (const auto& [basepath, basetype] : ItemsMap)
	{
		if (basetype == type)
		{
			if (ItemsPaths.count(basepath) == 0)
			{
				if (basepath == "GAME_HEALTH_BREADWATER")
					ItemsPaths[basepath] = PathManager::getImageSetPath("LEVEL_HEALTH");
				else
					ItemsPaths[basepath] = PathManager::getImageSetPath(basepath);
			}
			return ItemsPaths[basepath];
		}
	}

	return "";
}
int PathManager::getItemType(const string& imageSet)
{
	return ItemsMap.at(imageSet);
}

map<string, string>::iterator PathManager::findImageSet(const string& imageSet)
{
	return find_if(imageSetMap.begin(), imageSetMap.end(),
		[&](const pair<string, string>& p) { return startsWith(imageSet, p.first + '_'); }
	);
}
