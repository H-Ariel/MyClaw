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


Item::Item(const WwdObject& obj, int8_t type, bool isFromMap)
	: BaseDynamicPlaneObject(obj), _type((Type)type), _delayBeforeRespawn(0), _useGlitter(false)
{
	if (_type == Type::Default)
	{
		myMemCpy(_type, Type::Treasure_Coins);
	}
	else if (_type == Type::None)
	{
		myMemCpy(_type, (Type)PathManager::getItemType(obj.imageSet));
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

	_ani = AssetsManager::createAnimationFromDirectory(PathManager::getItemPath(_type, obj.imageSet));
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
		AssetsManager::playWavFile(path, 50);
		if (!strcmp(path, "GAME/SOUNDS/MAPPIECE.WAV"))
			Sleep(1520); // in case of map-piece sound, wait for the sound to finish
	}
}

Item* Item::getItem(const WwdObject& obj, bool isFromMap, int8_t type)
{
	Type _type((Type)type);
	// TODO: same code at Item-ctor. try remove one of them (probably here)
	if (_type == Type::Default)
	{
		_type = Type::Treasure_Coins;
	}
	if (_type == Type::None)
	{
		_type = (Type)PathManager::getItemType(obj.imageSet);
	}
	if (_type == Type::Warp || _type == Type::BossWarp)
	{
		return DBG_NEW::Warp(obj, _type);
	}
	return DBG_NEW Item(obj, _type, isFromMap);
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


Warp::Warp(const WwdObject& obj, int8_t type)
	: Item(obj, type, true), _destination({ (float)obj.speedX, (float)obj.speedY }),
	_oneTimeWarp(obj.smarts == 0), _activateWarp(false)
{
}
void Warp::Logic(uint32_t elapsedTime)
{
	if (GetRect().intersects(player->GetRect()))
	{
		removeObject = _oneTimeWarp;
		playItemSound();
		_activateWarp = true; // set the destination warp so ClawLevelEngine can teleport the player to it
	}
}

// deactivate
