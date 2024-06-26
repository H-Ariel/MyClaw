#include "Item.h"
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
const Warp* Warp::DestinationWarp = nullptr;


Item::Item(const WwdObject& obj, int8_t type, bool isFromMap)
	: BaseDynamicPlaneObject(obj), _type((Type)type), _delayBeforeRespawn(0), _useGlitter(false)
{
	if (_type == Type::Default)
	{
		(Type&)_type = Type::Treasure_Coins;
	}
	else if (_type == Type::None)
	{
		(Type&)_type = ItemsMap.at(obj.imageSet);
	}

	if (isFromMap && _type != Type::BossWarp && _type != Type::Warp && _type != Type::Treasure_Coins)
	{
		_useGlitter = true;
		_glitterAnimation = AssetsManager::createAnimationFromDirectory("GAME/IMAGES/GLITTER");
	}

	_respawning = obj.damage;

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

	_ani = AssetsManager::createAnimationFromDirectory(getItemPath(_type, obj.imageSet));
	_ani->updateFrames = FindInArray(UpdateFramesTypes, _type);

	if (drawZ == 0) // is that correct?
		drawZ = DefaultZCoord::Items;
}
void Item::Logic(uint32_t elapsedTime)
{
	if (speed.x == 0 && speed.y == 0)
	{
		if (_delayBeforeRespawn > 0)
			_delayBeforeRespawn -= elapsedTime;

		if (!_respawning || _delayBeforeRespawn <= 0)
		{
			if (GetRect().intersects(player->GetRect()))
			{
				if (player->collectItem(this))
				{
					playItemSound();
					removeObject = !_respawning;
					_delayBeforeRespawn = 10000; // respawn after 10 seconds
				}
			}
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
	if (_delayBeforeRespawn <= 0) // draw only if playr can take it
	{
		BaseDynamicPlaneObject::Draw();

		if (_useGlitter)
		{
			_glitterAnimation->position = position;
			_glitterAnimation->Draw();
		}
	}
}
void Item::stopFalling(float collisionSize)
{
	speed.y = 0;
	speed.x = 0;
	position.y -= collisionSize;
	_useGlitter = true;
	_glitterAnimation = AssetsManager::createAnimationFromDirectory("GAME/IMAGES/GLITTER");
}
int Item::getTreasureScore() const
{
	switch (_type)
	{
	case Default:
	case Treasure_Coins:
		return 100;

	case Treasure_Goldbars:
		return 500;

	case Treasure_Rings_Red:
	case Treasure_Rings_Green:
	case Treasure_Rings_Blue:
	case Treasure_Rings_Purple:
		return 1500;

	case Treasure_Necklace:
	case Treasure_Chalices_Red:
	case Treasure_Chalices_Green:
	case Treasure_Chalices_Blue:
	case Treasure_Chalices_Purple:
		return 2500;

	case Treasure_Crosses_Red:
	case Treasure_Crosses_Green:
	case Treasure_Crosses_Blue:
	case Treasure_Crosses_Purple:
		return 5000;

	case Treasure_Scepters_Red:
	case Treasure_Scepters_Green:
	case Treasure_Scepters_Blue:
	case Treasure_Scepters_Purple:
		return 7500;

	case Treasure_Geckos_Red:
	case Treasure_Geckos_Green:
	case Treasure_Geckos_Blue:
	case Treasure_Geckos_Purple:
		return 10000;

	case Treasure_Crowns_Red:
	case Treasure_Crowns_Green:
	case Treasure_Crowns_Blue:
	case Treasure_Crowns_Purple:
		return 15000;

	case Treasure_Skull_Red:
	case Treasure_Skull_Green:
	case Treasure_Skull_Blue:
	case Treasure_Skull_Purple:
		return 25000;
	}

	throw Exception("invalid treasure type");
}
void Item::playItemSound() const
{
	const char* path = nullptr;

	switch (_type)
	{
	case Default:
	case Treasure_Coins:
		path = "GAME/SOUNDS/COIN.WAV";
		break;

	case Treasure_Goldbars:
		path = "GAME/SOUNDS/TREASURE.WAV";
		break;

	case Treasure_Rings_Red:
	case Treasure_Rings_Green:
	case Treasure_Rings_Blue:
	case Treasure_Rings_Purple:
		path = "GAME/SOUNDS/RINGS.WAV";
		break;

	case Treasure_Necklace:
	case Treasure_Chalices_Red:
	case Treasure_Chalices_Green:
	case Treasure_Chalices_Blue:
	case Treasure_Chalices_Purple:
		path = "GAME/SOUNDS/PICKUP1.WAV";
		break;

	case Treasure_Crosses_Red:
	case Treasure_Crosses_Green:
	case Treasure_Crosses_Blue:
	case Treasure_Crosses_Purple:
		path = "GAME/SOUNDS/CROSS.WAV";
		break;

	case Treasure_Scepters_Red:
	case Treasure_Scepters_Green:
	case Treasure_Scepters_Blue:
	case Treasure_Scepters_Purple:
		path = "GAME/SOUNDS/SCEPTER.WAV";
		break;

	case Treasure_Geckos_Red:
	case Treasure_Geckos_Green:
	case Treasure_Geckos_Blue:
	case Treasure_Geckos_Purple:
		path = "GAME/SOUNDS/PICKUP2.WAV";
		break;

	case Treasure_Crowns_Red:
	case Treasure_Crowns_Green:
	case Treasure_Crowns_Blue:
	case Treasure_Crowns_Purple:
		path = "GAME/SOUNDS/PICKUP1.WAV";
		break;

	case Treasure_Skull_Red:
	case Treasure_Skull_Green:
	case Treasure_Skull_Blue:
	case Treasure_Skull_Purple:
		path = "GAME/SOUNDS/PICKUP1.WAV";
		break;

	case Ammo_Deathbag:
	case Ammo_Shot:
	case Ammo_Shotbag:
		path = "GAME/SOUNDS/AMMUNITION.WAV";
		break;

	case Ammo_Magic_5:
	case Ammo_Magic_10:
	case Ammo_Magic_25:
		path = "GAME/SOUNDS/MAGICPOWERUP.WAV";
		break;

	case Health_Level:
		path = "GAME/SOUNDS/FOODITEM.WAV";
		break;

	case Health_25:
	case Health_10:
	case Health_15:
		path = "GAME/SOUNDS/MILK.WAV";
		break;

	case MapPiece:
	case NineLivesGem:
		path = "GAME/SOUNDS/MAPPIECE.WAV";
		break;

	case Warp:
	case BossWarp:
		path = "GAME/SOUNDS/WARP.WAV";
		break;

	case Powerup_FireSword:
		path = "CLAW/SOUNDS/1110001.WAV";
		break;
	case Powerup_LightningSword:
		path = "CLAW/SOUNDS/1110022.WAV";
		break;
	case Powerup_IceSword:
		path = "CLAW/SOUNDS/1110012.WAV";
		break;
	case Powerup_ExtraLife:
		path = "GAME/SOUNDS/EXTRALIFE.WAV";
		break;

		/*
	case Ammo_Dynamite:
		break;

	case Powerup_Catnip_White:
		break;
	case Powerup_Catnip_Red:
		break;
	case Powerup_Invisibility:
		break;
	case Powerup_Invincibility:
		break;

	case Curse_Ammo:
		break;
	case Curse_Magic:
		break;
	case Curse_Health:
		break;
	case Curse_Life:
		break;
	case Curse_Treasure:
		break;
	case Curse_Freeze:
		break;
		*/

	default:
		break;
	}

	if (path)
	{
		AssetsManager::playWavFile(path);
		if (!strcmp(path, "GAME/SOUNDS/MAPPIECE.WAV"))
			Sleep(1520); // wait until the sound ends
	}
}

Item* Item::getItem(const WwdObject& obj, bool isFromMap, int8_t type)
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
	return DBG_NEW Item(obj, type, isFromMap);
}
Item* Item::getItem(const WwdObject& obj, bool isFromMap)
{
	return getItem(obj, isFromMap, Type::None);
}
Item* Item::getItem(const WwdObject& obj, int8_t type)
{
	return getItem(obj, false, type);
}
Item* Item::getItem(const WwdObject& obj)
{
	return getItem(obj, false, Type::None);
}
void Item::resetItemsPaths()
{
	ItemsPaths.erase("LEVEL_GEM");
	ItemsPaths.erase("LEVEL_HEALTH");
	ItemsPaths.erase("GAME_HEALTH_BREADWATER");
}
string Item::getItemPath(Type type, const string& imageSet)
{
	if (type == Type::Warp)
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
				{
					ItemsPaths[basepath] = PathManager::getImageSetPath("LEVEL_HEALTH");
				}
				else
				{
					ItemsPaths[basepath] = PathManager::getImageSetPath(basepath);
				}
			}
			return ItemsPaths[basepath];
		}
	}

	return "";
}


Warp::Warp(const WwdObject& obj, int8_t type)
	: Item(obj, type, true), _destination({ (float)obj.speedX, (float)obj.speedY }), _oneTimeWarp(obj.smarts == 0)
{
}
void Warp::Logic(uint32_t elapsedTime)
{
	if (GetRect().intersects(player->GetRect()))
	{
		removeObject = _oneTimeWarp;
		playItemSound();
		DestinationWarp = this; // set the destination warp so ClawLevelEngine can teleport the player to it
	}
}
