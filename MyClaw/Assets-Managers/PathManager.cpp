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

string PathManager::getImageSetPath(const string& _imageSet)
{
	if (data[IMAGE_SET].count(_imageSet) == 0)
	{
		string newImageSet(_imageSet);

		if (!contains(newImageSet, "IMAGES"))
			for (const auto& [prefix, imageSet] : imageSetMap)
				if (startsWith(newImageSet, prefix + '_'))
				{
					newImageSet = replaceFirst(newImageSet, prefix, imageSet);
					break;
				}

		newImageSet = replaceString(newImageSet, '_', '/');
		data[IMAGE_SET][_imageSet] = newImageSet;
	}

	return data[IMAGE_SET][_imageSet];
}
string PathManager::getAnimationSetPath(const string& _aniSet)
{
	if (data[ANIMATION_SET].count(_aniSet) == 0)
	{
		string aniSet(_aniSet);

		for (const auto& [prefix, imageSet] : imageSetMap)
		{
			if (startsWith(aniSet, prefix + '_'))
			{
				aniSet = replaceFirst(aniSet, prefix, replaceFirst(imageSet, "IMAGES", "ANIS"));
				break;
			}
		}

		aniSet = replaceString(aniSet, '_', '/');
		if (aniSet == "LEVEL12/ANIS/KINGAQUATIS")
			aniSet = "LEVEL12/ANIS/AQUATIS";
		data[ANIMATION_SET][_aniSet] = aniSet;
	}

	return data[ANIMATION_SET][_aniSet];
}
string PathManager::getAnimationPath(const string& _path)
{
	if (data[ANIMATION].count(_path) == 0)
	{
		string path(_path);
		path = getAnimationSetPath(path);
		path += ".ANI";
		// TODO: hack - something else
		if (path == "LEVEL1/ANIS/MANICALS/MANICAL.ANI" || path == "LEVEL1/ANIS/MANICALS/M.ANI")
		{
			path = "LEVEL1/ANIS/MANICLES/MANICAL.ANI";
		}

		data[ANIMATION][_path] = path;
	}

	return data[ANIMATION][_path];
}
string PathManager::getSoundFilePath(const string& _path)
{
	if (data[SOUND].count(_path) == 0)
	{
		string path(_path);

		// TODO: hack - something else
		if (imageSetMap["LEVEL"] == "LEVEL1" && path == "LEVEL_AMBIENT_ANGVIL")
		{
			path = "LEVEL_AMBIENT_ANVIL";
		}
		else if (imageSetMap["LEVEL"] == "LEVEL3_IMAGES")
		{
			if (path == "LEVEL_TRIGGER_1013")
			{
				path = "LEVEL_TRIGGER_1012";
			}
			else if (path == "LEVEL_TRIGGER_BIRDCALL2")
			{
				path = "LEVEL_AMBIENT_BIRDCALL2";
			}
		}

		path = replaceFirst(path, "CLAW", "CLAW/SOUNDS");

		for (const auto& [prefix, imageSet] : imageSetMap)
			if (startsWith(path, prefix + '_'))
			{
				path = replaceFirst(path, prefix, replaceFirst(imageSet, "IMAGES", "SOUNDS"));
				break;
			}

		path = replaceString(path, '_', '/');
		data[SOUND][_path] = path + ".WAV";
	}

	return data[SOUND][_path];
}
string PathManager::getBackgroundMusicFilePath(const string& _path)
{
	if (data[BACKGROUND_MUSIC].count(_path) == 0)
	{
		string path(_path);

		for (const auto& [prefix, imageSet] : imageSetMap)
			if (startsWith(path, prefix + '_'))
			{
				path = replaceFirst(path, prefix, replaceFirst(imageSet, "IMAGES", "MUSIC"));
				break;
			}

		path = replaceString(path, '_', '/');
		data[BACKGROUND_MUSIC][_path] = path + ".XMI";
	}

	return data[BACKGROUND_MUSIC][_path];
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
