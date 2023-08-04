#include "Item.h"
#include "../Assets-Managers/AssetsManager.h"
#include "../Player.h"
#include "../ActionPlane.h"


const initializer_list<Item::Type> Item::UpdateFramesTypes = {
	Warp,
	Treasure_Coins,
	Curse_Ammo,
	Curse_Magic,
	Curse_Health,
	Curse_Life,
	Curse_Treasure,
	Curse_Freeze,
	Powerup_Invisibility,
	Powerup_Invincibility,
	Powerup_ExtraLife,
	Powerup_FireSword,
	Powerup_LightningSword,
	Powerup_IceSword,
	BossWarp,
};
const map<string, Item::Type> Item::ItemsMap = {
	{ "GAME_TREASURE_GOLDBARS", Treasure_Goldbars },
	{ "GAME_TREASURE_RINGS_RED", Treasure_Rings_Red },
	{ "GAME_TREASURE_RINGS_GREEN", Treasure_Rings_Green },
	{ "GAME_TREASURE_RINGS_BLUE", Treasure_Rings_Blue },
	{ "GAME_TREASURE_RINGS_PURPLE", Treasure_Rings_Purple },
	{ "GAME_TREASURE_NECKLACE", Treasure_Necklace },
	{ "GAME_TREASURE_CROSSES_RED", Treasure_Crosses_Red },
	{ "GAME_TREASURE_CROSSES_GREEN", Treasure_Crosses_Green },
	{ "GAME_TREASURE_CROSSES_BLUE", Treasure_Crosses_Blue },
	{ "GAME_TREASURE_CROSSES_PURPLE", Treasure_Crosses_Purple },
	{ "GAME_TREASURE_SCEPTERS_RED", Treasure_Scepters_Red },
	{ "GAME_TREASURE_SCEPTERS_GREEN", Treasure_Scepters_Green },
	{ "GAME_TREASURE_SCEPTERS_BLUE", Treasure_Scepters_Blue },
	{ "GAME_TREASURE_SCEPTERS_PURPLE", Treasure_Scepters_Purple },
	{ "GAME_TREASURE_GECKOS_RED", Treasure_Geckos_Red },
	{ "GAME_TREASURE_GECKOS_GREEN", Treasure_Geckos_Green },
	{ "GAME_TREASURE_GECKOS_BLUE", Treasure_Geckos_Blue },
	{ "GAME_TREASURE_GECKOS_PURPLE", Treasure_Geckos_Purple },
	{ "GAME_AMMO_DEATHBAG", Ammo_Deathbag },
	{ "GAME_AMMO_SHOT", Ammo_Shot },
	{ "GAME_AMMO_SHOTBAG", Ammo_Shotbag },
	{ "GAME_CATNIPS_NIP1", Powerup_Catnip_White },
	{ "GAME_CATNIPS_NIP2", Powerup_Catnip_Red },
	{ "LEVEL_HEALTH", Health_Level },
	{ "GAME_HEALTH_BREADWATER", Health_Level },
	{ "GAME_HEALTH_POTION3", Health_25 },
	{ "GAME_HEALTH_POTION1", Health_10 },
	{ "GAME_HEALTH_POTION2", Health_15 },
	{ "GAME_MAGIC_GLOW", Ammo_Magic_5 },
	{ "GAME_MAGIC_STARGLOW", Ammo_Magic_10 },
	{ "GAME_MAGIC_MAGICCLAW", Ammo_Magic_25 },
	{ "GAME_MAPPIECE", MapPiece },
	{ "GAME_WARP", Warp },
	{ "GAME_VERTWARP", Warp },
	{ "GAME_TREASURE_COINS", Treasure_Coins },
	{ "GAME_DYNAMITE", Ammo_Dynamite },
	{ "GAME_CURSES_AMMO", Curse_Ammo },
	{ "GAME_CURSES_MAGIC", Curse_Magic },
	{ "GAME_CURSES_HEALTH", Curse_Health },
	{ "GAME_CURSES_LIFE", Curse_Life },
	{ "GAME_CURSES_TREASURE", Curse_Treasure },
	{ "GAME_CURSES_FREEZE", Curse_Freeze },
	{ "GAME_TREASURE_CHALICES_RED", Treasure_Chalices_Red },
	{ "GAME_TREASURE_CHALICES_GREEN", Treasure_Chalices_Green },
	{ "GAME_TREASURE_CHALICES_BLUE", Treasure_Chalices_Blue },
	{ "GAME_TREASURE_CHALICES_PURPLE", Treasure_Chalices_Purple },
	{ "GAME_TREASURE_CROWNS_RED", Treasure_Crowns_Red },
	{ "GAME_TREASURE_CROWNS_GREEN", Treasure_Crowns_Green },
	{ "GAME_TREASURE_CROWNS_BLUE", Treasure_Crowns_Blue },
	{ "GAME_TREASURE_CROWNS_PURPLE", Treasure_Crowns_Purple },
	{ "GAME_TREASURE_JEWELEDSKULL_RED", Treasure_Skull_Red },
	{ "GAME_TREASURE_JEWELEDSKULL_GREEN", Treasure_Skull_Green },
	{ "GAME_TREASURE_JEWELEDSKULL_BLUE", Treasure_Skull_Blue },
	{ "GAME_TREASURE_JEWELEDSKULL_PURPLE", Treasure_Skull_Purple },
	{ "GAME_POWERUPS_GHOST", Powerup_Invisibility },
	{ "GAME_POWERUPS_INVULNERABLE", Powerup_Invincibility },
	{ "GAME_POWERUPS_EXTRALIFE", Powerup_ExtraLife },
	{ "GAME_POWERUPS_LIGHTNINGSWORD", Powerup_LightningSword },
	{ "GAME_POWERUPS_FIRESWORD", Powerup_FireSword },
	{ "GAME_POWERUPS_ICESWORD", Powerup_IceSword },
	{ "GAME_BOSSWARP", BossWarp },
	{ "LEVEL_GEM", NineLivesGem }
};
map<string, string> Item::ItemsPaths;

string Item::getItemPath(Type type, const string& imageSet)
{
	if (type == Type::Warp)
	{
		// because we have WARP and VERTWARP
		return PathManager::getImageSetPath(imageSet);
	}

	for (auto& i : ItemsMap)
	{
		if (i.second == type)
		{
			if (ItemsPaths.count(i.first) == 0)
			{
				if (i.first == "LEVEL_HEALTH" || i.first == "GAME_HEALTH_BREADWATER")
				{
					ItemsPaths[i.first] = PathManager::getImageSetPath("LEVEL_HEALTH");
				}
				else
				{
					ItemsPaths[i.first] = PathManager::getImageSetPath(i.first);
				}
			}
			return ItemsPaths[i.first];
		}
	}

	return "";
}

// TODO: activate WAV sound when item is picked up

Item::Item(const WwdObject& obj, int8_t type)
	: BaseDynamicPlaneObject(obj), _type((Type)type), _useGlitter(false), _glitterAnimation(nullptr)
{
	if (_type == Type::Default)
	{
		(Type&)_type = Type::Treasure_Coins;
	}
	else if (_type == Type::None)
	{
		(Type&)_type = ItemsMap.at(obj.imageSet);
	}

#ifndef LOW_DETAILS
	if (_type != Item::Type::BossWarp && _type != Item::Type::Warp && _type != Item::Type::Treasure_Coins)
	{
		_useGlitter = true;
		_glitterAnimation = AssetsManager::createAnimationFromDirectory("GAME/IMAGES/GLITTER", 125, false);
	}
#endif

	_duration = obj.smarts;
	if (_duration == 0)
	{
		switch (_type)
		{
		case Powerup_Catnip_White:
			_duration = 15000;
			break;

		case Powerup_Invisibility:
		case Powerup_Invincibility:
		case Powerup_FireSword:
		case Powerup_LightningSword:
		case Powerup_IceSword:
		case Powerup_Catnip_Red:
			_duration = 30000;
			break;
		}
	}


	_ani = AssetsManager::createAnimationFromDirectory(getItemPath(_type, obj.imageSet), 125, false);
	_ani->updateFrames = FindInArray(UpdateFramesTypes, _type);
}
void Item::Logic(uint32_t elapsedTime)
{
	if (speed.x == 0 && speed.y == 0)
	{
		if (GetRect().intersects(player->GetRect()))
		{
			// if the player collect the item it will be removed
			removeObject = player->collectItem(this);
		}
	}

	if (speed.y != 0)
	{
		speed.y += GRAVITY * elapsedTime;
		position.y += speed.y * elapsedTime;
	}
	if (speed.x != 0)
	{
		position.x += speed.x * elapsedTime;
	}
}
void Item::Draw()
{
	BaseDynamicPlaneObject::Draw();

	if (_useGlitter)
	{
		_glitterAnimation->position = position;
		_glitterAnimation->updateImageData();
		_glitterAnimation->Draw();
	}
}
void Item::stopFalling(float collisionSize)
{
	speed.y = 0;
	speed.x = 0;
	position.y -= collisionSize;
}

Item* Item::getItem(const WwdObject& obj, int8_t type)
{
	Type _type((Type)type);
	if (_type == Type::Default)
	{
		_type = Type::Treasure_Coins;
	}
	if (_type == Type::None)
	{
		_type = ItemsMap.at(obj.imageSet);
	}

	if (_type == Type::Warp || _type == Type::BossWarp)
	{
		return DBG_NEW ::Warp(obj, type);
	}

	return DBG_NEW Item(obj, type);
}
void Item::resetItemsPaths()
{
	ItemsPaths.clear();
}
uint32_t Item::getTreasureScore(Type type)
{
	switch (type)
	{
	case Item::Default:
	case Item::Treasure_Coins:
		return 100;

	case Item::Treasure_Goldbars:
		return 500;
		
	case Item::Treasure_Rings_Red:
	case Item::Treasure_Rings_Green:
	case Item::Treasure_Rings_Blue:
	case Item::Treasure_Rings_Purple:
		return 1500;
		
	case Item::Treasure_Necklace:
	case Item::Treasure_Chalices_Red:
	case Item::Treasure_Chalices_Green:
	case Item::Treasure_Chalices_Blue:
	case Item::Treasure_Chalices_Purple:
		return 2500;
		
	case Item::Treasure_Crosses_Red:
	case Item::Treasure_Crosses_Green:
	case Item::Treasure_Crosses_Blue:
	case Item::Treasure_Crosses_Purple:
		return 5000;
		
	case Item::Treasure_Scepters_Red:
	case Item::Treasure_Scepters_Green:
	case Item::Treasure_Scepters_Blue:
	case Item::Treasure_Scepters_Purple:
		return 7500;
		
	case Item::Treasure_Geckos_Red:
	case Item::Treasure_Geckos_Green:
	case Item::Treasure_Geckos_Blue:
	case Item::Treasure_Geckos_Purple:
		return 10000;
		
	case Item::Treasure_Crowns_Red:
	case Item::Treasure_Crowns_Green:
	case Item::Treasure_Crowns_Blue:
	case Item::Treasure_Crowns_Purple:
		return 15000;
		
	case Item::Treasure_Skull_Red:
	case Item::Treasure_Skull_Green:
	case Item::Treasure_Skull_Blue:
	case Item::Treasure_Skull_Purple:
		return 25000;
	}

	throw Exception("invalid treasure type");
}

Warp::Warp(const WwdObject& obj, int8_t type)
	: Item(obj, type), _destination({ (float)obj.speedX, (float)obj.speedY }), _oneTimeWarp(obj.smarts == 0)
{
}
void Warp::Logic(uint32_t elapsedTime)
{
	if (GetRect().intersects(player->GetRect()))
	{
		player->position = _destination;
		player->stopFalling(0);
		removeObject = _oneTimeWarp;

		if (_type == Type::BossWarp)
		{
			ActionPlane::playerEnterToBoss();
			player->startPosition = _destination;
		}

		// TODO: cool animation
	}
}
