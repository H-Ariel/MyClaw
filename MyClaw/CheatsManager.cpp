#include "CheatsManager.h"
#include "ActionPlane.h"
#include "Objects/Item.h"
#include "GameEngine/WindowManager.h"


#define CHEATS_PREFIX "MP" // Monolith Production (the original game) prefix
#define CHEATS_PREFIX_SIZE 2

#define MODE_CHANGE_MSG "%s mode is %s"


// TODO: add more cheats
// maybe use the original game's cheats codes


// The list of cheats, format: { type, keys, message }
// NOTE: make sure cheat-code contains only uppercase letters
// AH - my own cheats. it means Ariel Halili (me) :)
const vector<tuple<int, const char*, const char*>> CheatsManager::cheatsKeys({
	{ FireSword		, "HOTSTUFF", "Fire sword rules..." },
	{ IceSword		, "PENGUIN"	, "Ice sword rules..." },
	{ LightningSword, "FRANKLIN", "Lightning sword rules..." },
	{ Catnip		, "FREAK"	, "Catnip...Yummy!" },
	{ Invisibility	, "CASPER"	, "Now you see me... now you dont!" },
	{ Invincibility	, "VADER"	, "Sticks and stones won't break my bones..." },
	{ FillHealth	, "APPLE"	, "Full Health" },
	{ FillPistol	, "LOADED"	, "Full Ammo" },
	{ FillMagic		, "GANDOLF"	, "Full Magic" },
	{ FillDynamite	, "BLASTER"	, "Full Dynamite" },
	{ FillLife		, "AHCAKE"	, "Full Life" },
	{ FinishLevel	, "SCULLY"	, "Finish Level" },
	{ GodMode		, "KFA"		, MODE_CHANGE_MSG },
	{ EasyMode		, "EASYMODE", MODE_CHANGE_MSG },
	{ SuperStrong	, "BUNZ"	, MODE_CHANGE_MSG },
	{ Flying		, "FLY"		, MODE_CHANGE_MSG },
	{ SuperJump		, "JORDAN"	, MODE_CHANGE_MSG },
	{ BgMscSpeedUp	, "MAESTRO"	, "Time to speed things up!" },
	{ BgMscSlowDown	, "LANGSAM"	, "Ready to slow things down?" },
	{ BgMscNormal	, "NORMALMUSIC", "Back to normal..." },
	{ MultiTeasures	, "AHGOLD"	, MODE_CHANGE_MSG }, // gives more treasures at crates
	{ IncResolution	, "INCVID", "Resolution increased" },
	{ DecResolution	, "DECVID", "Resolution decreased" },
	{ DefaultResolution, "DEFVID", "Default resolution" }
});


CheatsManager::CheatsManager()
	: _god(false), _superStrong(false), _flying(false),
	_easy(false), _superJump(false), _multiTreasures(false)
{
#ifdef _DEBUG
	_god = true;
	_superJump = true;
#endif
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
	case FinishLevel:	BasePlaneObject::player->cheat(type); break;
		//	case BgMscSpeedUp:	if (MidiPlayer::MusicSpeed < 2) MidiPlayer::MusicSpeed += 0.25f; break;
		//	case BgMscSlowDown:	if (MidiPlayer::MusicSpeed > 0.25f) MidiPlayer::MusicSpeed -= 0.25f; break;
		//	case BgMscNormal:	MidiPlayer::MusicSpeed = 1; break;

	case GodMode:		_god = !_god; break;
	case EasyMode:		_easy = !_easy; break;
	case SuperStrong:	_superStrong = !_superStrong; break;
	case Flying:
		if (BasePlaneObject::player->cheat(type)) // try to enable flying mode
			_flying = !_flying;
		break;
	case SuperJump:		_superJump = !_superJump; break;
	case MultiTeasures: _multiTreasures = !_multiTreasures; break;

	case IncResolution: WindowManager::setWindowScale(WindowManager::getWindowScale() + 0.5); break;
	case DecResolution: WindowManager::setWindowScale(WindowManager::getWindowScale() - 0.5); break;
	case DefaultResolution: WindowManager::setDefaultWindowScale(); break;

	default: break;
	}
}

int CheatsManager::getCheatType()
{
	int keysSize = (int)keys.size() - CHEATS_PREFIX_SIZE;

	// check if keys starts with the prefix
	if (keysSize <= 0 || strncmp(keys.data(), CHEATS_PREFIX, CHEATS_PREFIX_SIZE))
		return Types::None;

	for (const auto& [type, cheatKey, msg] : cheatsKeys) {
		size_t cheatKeySize = strlen(cheatKey);

		if (keysSize == cheatKeySize && !strncmp(&keys[2], cheatKey, cheatKeySize)) {
			keys.clear();
			char temp[32] = {};
			if (msg == MODE_CHANGE_MSG) {
				bool status = false;
				const char* mode = nullptr;
				switch (type) {
				case GodMode:		status = _god;			mode = "God";		break;
				case EasyMode:		status = _easy;			mode = "Easy";		break;
				case SuperStrong:	status = _superStrong;	mode = "Roids";		break;
				case Flying:		status = _flying;		mode = "Flying";	break;
				case SuperJump:		status = _superJump;	mode = "Super Jump"; break;
				case MultiTeasures: status = _multiTreasures; mode = "Multi Ttreasures"; break;
				default: mode = "<Unknown>"; break;
				}
				sprintf(temp, msg, mode, status ? "off" : "on");
			}
			ActionPlane::writeMessage(temp[0] ? temp : msg);
			return type;
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
