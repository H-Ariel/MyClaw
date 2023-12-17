#include "CheatsManager.h"
#include "ActionPlane.h"
#include "Objects/Item.h"


// TODO: add more cheats
// maybe use the original game's cheats codes


enum CheatType
{
	None,
	FireSword,
	IceSword,
	LightningSword,
	Catnip,
	Invisibility,
	Invincibility
};


CheatsManager::CheatsManager()
	: cheatKeys({
		{ CheatType::FireSword, "FIRE", "Fire sword rules..." },
		{ CheatType::IceSword, "ICE", "Ice sword rules..." },
		{ CheatType::LightningSword, "LIGHT", "Lightning sword rules..." },
		{ CheatType::Catnip, "CATNIP", "Catnip...Yummy!" },
		{ CheatType::Invisibility, "VISI", "Now you see me...now you dont!" },
		{ CheatType::Invincibility, "VINCI", "Sticks and stones won't break my bones..." },
	})
{
	// The list of cheats, format: { type, keys, message }
	// NOTE: make sure cheat-code contains only uppercase letters
}

void CheatsManager::addKey(int key)
{
	if (!isalpha(key))
	{
		keys.clear();
		return;
	}

	keys.push_back((char)key);

	switch (getCheatType())
	{
	case CheatType::FireSword:		addPowerup(Item::Type::Powerup_FireSword); break;
	case CheatType::IceSword:		addPowerup(Item::Type::Powerup_IceSword); break;
	case CheatType::LightningSword:	addPowerup(Item::Type::Powerup_LightningSword); break;
	case CheatType::Catnip:			addPowerup(Item::Type::Powerup_Catnip_White); break;
	case CheatType::Invisibility:	addPowerup(Item::Type::Powerup_Invisibility); break;
	case CheatType::Invincibility:	addPowerup(Item::Type::Powerup_Invincibility); break;

	default: break;
	}
}

int CheatsManager::getCheatType()
{
	size_t keysSize = keys.size();

	for (const auto& [type, cheatKeys, msg] : cheatKeys)
	{
		size_t cheatKeysSize = strlen(cheatKeys);

		if (keysSize >= cheatKeysSize)
		{
			bool match = true;
			for (int i = 0; match && i < cheatKeysSize; i++)
				match = (keys[keysSize - i - 1] == cheatKeys[cheatKeysSize - i - 1]);

			if (match)
			{
				keys.clear();
				ActionPlane::writeMessage(msg);
				return type;
			}
		}
	}

	return CheatType::None;
}

void CheatsManager::addPowerup(int8_t powerupType)
{
	WwdObject obj;
	obj.smarts = 30000;
	Item* item = Item::getItem(obj, powerupType);
	item->position = BasePlaneObject::player->position;
	BasePlaneObject::player->collectItem(item);
	delete item;
}
