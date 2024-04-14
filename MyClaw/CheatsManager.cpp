#include "CheatsManager.h"
#include "ActionPlane.h"
#include "Objects/Item.h"


// TODO: add more cheats
// maybe use the original game's cheats codes


CheatsManager::CheatsManager()
	: cheatKeys({
		{ FireSword		, "MPHOTSTUFF"	, "Fire sword rules..." },
		{ IceSword		, "MPPENGUIN"	, "Ice sword rules..." },
		{ LightningSword, "MPFRANKLIN"	, "Lightning sword rules..." },
		{ Catnip		, "MPFREAK"		, "Catnip...Yummy!" },
		{ Invisibility	, "MPCASPER"	, "Now you see me...now you dont!" },
		{ Invincibility	, "MPVADER"		, "Sticks and stones won't break my bones..." },
		{ FillHealth	, "MPAPPLE"		, "Full Health" },
		{ FillPistol	, "MPLOADED"	, "Full Ammo" },
		{ FillMagic		, "MPGANDOLF"	, "Full Magic" },
		{ FillDynamite	, "MPBLASTER"	, "Full Dynamite" },
		{ FillLife		, "AHCAKE"		, "Full Life" },
		{ FinishLevel	, "MPSCULLY"	, "Finish Level" },
		{ BgMscSpeedUp	, "MPMAESTRO"	, "Time to speed things up!" },
		{ BgMscSlowDown	, "MPLANGSAM"	, "Ready to slow things down?" },
		{ BgMscNormal	, "MPNORMALMUSIC", "Back to normal..." }
	})
{
	// The list of cheats, format: { type, keys, message }
	// NOTE: make sure cheat-code contains only uppercase letters
	// MP - Monolith Production (the original game) ; AH - Ariel Halili (me)
}

void CheatsManager::addKey(int key)
{
	if (!isalpha(key))
	{
		keys.clear();
		return;
	}

	keys.push_back((char)key);

	int type = getCheatType();
	switch (type)
	{
	case FireSword:			addPowerup(Item::Type::Powerup_FireSword); break;
	case IceSword:			addPowerup(Item::Type::Powerup_IceSword); break;
	case LightningSword:	addPowerup(Item::Type::Powerup_LightningSword); break;
	case Catnip:			addPowerup(Item::Type::Powerup_Catnip_White); break;
	case Invisibility:		addPowerup(Item::Type::Powerup_Invisibility); break;
	case Invincibility:		addPowerup(Item::Type::Powerup_Invincibility); break;
	case FillHealth:
	case FillPistol:
	case FillMagic:
	case FillDynamite:
	case FillLife:
	case FinishLevel:		BasePlaneObject::player->cheat(type); break;
//	case BgMscSpeedUp:	if (MidiPlayer::MusicSpeed < 2) MidiPlayer::MusicSpeed += 0.25f; break;
//	case BgMscSlowDown:	if (MidiPlayer::MusicSpeed > 0.25f) MidiPlayer::MusicSpeed -= 0.25f; break;
//	case BgMscNormal:	MidiPlayer::MusicSpeed = 1; break;

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

	return Types::None;
}

void CheatsManager::addPowerup(int8_t powerupType)
{
	WwdObject obj;
	obj.smarts = 30000;
	Item* item = Item::getItem(obj, powerupType);
	item->position = BasePlaneObject::player->position;
	BasePlaneObject::player->collectItem(item);
	item->playItemSound();
	delete item;
}
