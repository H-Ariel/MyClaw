#include "CheatsManager.h"
#include "ActionPlane.h"
#include "GUI/ActionPlaneMessage.h"
#include "Objects/Item.h"


// TODO: add more cheats
// maybe use the original game's cheats codes


#define player BasePlaneObject::player


// converts a string to a vector of `int`s.
// NOTE: make sure `str` is null-terminated and contains only uppercase letters
vector<int> vectorFromString(const char* str)
{
	return vector<int>(str, str + strlen(str));
}

enum CheatType
{
	None,
	FireSword,
	IceSword,
	Catnip
};



vector<int> CheatsManager::keys;
map<int, vector<int>> CheatsManager::cheatKeys = {
	{ CheatType::FireSword, vectorFromString("FIRE") },
	{ CheatType::IceSword, vectorFromString("ICE") },
	{ CheatType::Catnip, vectorFromString("CATNIP") }
};


void CheatsManager::addKey(int key)
{
	if (!isalpha(key))
	{
		keys.clear();
		return;
	}

	keys.push_back(key);

	switch (getCheatType())
	{
	case CheatType::FireSword:	fireSword(); break;
	case CheatType::IceSword:	iceSword(); break;
	case CheatType::Catnip:		catnip(); break;

	default: break;
	}
}

int CheatsManager::getCheatType()
{
	size_t keysSize = keys.size();

	for (auto& [type, cheatKeys] : cheatKeys)
	{
		size_t cheatKeysSize = cheatKeys.size();

		if (keysSize >= cheatKeysSize)
		{
			bool match = true;
			for (int i = 0; match && i < cheatKeysSize; i++)
				match = (keys[keysSize - i - 1] == cheatKeys[cheatKeysSize - i - 1]);

			if (match)
			{
				keys.clear();
				return type;
			}
		}
	}

	return CheatType::None;
}

inline void CheatsManager::fireSword()
{
	addPowerup(Item::Type::Powerup_FireSword);
	ActionPlane::addPlaneObject(DBG_NEW ActionPlaneMessage(L"Fire sword rules..."));
}
inline void CheatsManager::iceSword()
{
	addPowerup(Item::Type::Powerup_IceSword);
	ActionPlane::addPlaneObject(DBG_NEW ActionPlaneMessage(L"Ice sword rules..."));
}
inline void CheatsManager::catnip()
{
	addPowerup(Item::Type::Powerup_Catnip_White);
	ActionPlane::addPlaneObject(DBG_NEW ActionPlaneMessage(L"Catnip...Yummy!"));
}

void CheatsManager::addPowerup(int8_t powerupType)
{
	WwdObject obj;
	obj.smarts = 30000;
	Item* item = Item::getItem(obj, powerupType);
	item->position = player->position;
	player->collectItem(item);
	delete item;
}
