#include "CheatsManager.h"
#include "ActionPlane.h"
#include "Objects/Item.h"
#include "GameEngine/WindowManager.h"
#include "GlobalObjects.h"


#define MODE_CHANGE_MSG "%s mode is %s"


// The list of cheats, format: { type, keys, message }
// NOTE: make sure cheat-code contains only uppercase letters
// MP - Monolith Production (the original game) prefix
// AH - my own cheats (it means Ariel Halili (me) :) )
const tuple<int, const char*, const char*> CheatsManager::cheatsKeys[] = {
	{ FireSword		, "MPHOTSTUFF"	, "Fire sword rules..." },
	{ IceSword		, "MPPENGUIN"	, "Ice sword rules..." },
	{ LightningSword, "MPFRANKLIN"	, "Lightning sword rules..." },
	{ Catnip		, "MPFREAK"		, "Catnip...Yummy!" },
	{ Invisibility	, "MPCASPER"	, "Now you see me... now you dont!" },
	{ Invincibility	, "MPVADER"		, "Sticks and stones won't break my bones..." },
	{ FillHealth	, "MPAPPLE"		, "Full Health" },
	{ FillPistol	, "MPLOADED"	, "Full Ammo" },
	{ FillMagic		, "MPGANDOLF"	, "Full Magic" },
	{ FillDynamite	, "MPBLASTER"	, "Full Dynamite" },
	{ FillLife		, "AHCAKE"		, "Full Life" },
	{ FinishLevel	, "MPSCULLY"	, "Finish Level" },
	{ GodMode		, "MPKFA"		, MODE_CHANGE_MSG },
	{ EasyMode		, "MPEASYMODE"	, MODE_CHANGE_MSG },
	{ SuperStrong	, "MPBUNZ"		, MODE_CHANGE_MSG },
	{ Flying		, "MPFLY"		, MODE_CHANGE_MSG },
	{ SuperJump		, "MPJORDAN"	, MODE_CHANGE_MSG },
	{ BgMscSpeedUp	, "MPMAESTRO"	, "Time to speed things up!" },
	{ BgMscSlowDown	, "MPLANGSAM"	, "Ready to slow things down?" },
	{ BgMscNormal	, "MPNORMALMUSIC", "Back to normal..." },
	{ MultiTeasures	, "AHGOLD"		, MODE_CHANGE_MSG }, // gives more treasures at crates
	{ IncResolution	, "MPINCVID"	, "Resolution increased" },
	{ DecResolution	, "MPDECVID"	, "Resolution decreased" },
	{ DefaultResolution, "MPDEFVID"	, "Default resolution" }
};


CheatsManager::CheatsManager(ActionPlane* actionPlane)
	: actionPlane(actionPlane), _god(false), _superStrong(false), _flying(false),
	_easy(false), _superJump(false), _multiTreasures(false)
{
#ifdef _DEBUG
	_god = true;
	_superStrong = true;
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
	case GodMode:		_god = !_god; // NO BREAKE HERE!
	case FillHealth:
	case FillPistol:
	case FillMagic:
	case FillDynamite:
	case FillLife:
	case FinishLevel:	GO::player->cheat(type); break;
		//	case BgMscSpeedUp:	if (MidiPlayer::MusicSpeed < 2) MidiPlayer::MusicSpeed += 0.25f; break;
		//	case BgMscSlowDown:	if (MidiPlayer::MusicSpeed > 0.25f) MidiPlayer::MusicSpeed -= 0.25f; break;
		//	case BgMscNormal:	MidiPlayer::MusicSpeed = 1; break;

	//case GodMode:		_god = !_god; break;
	case EasyMode:		_easy = !_easy;
		if (_easy) actionPlane->enterEasyMode();
		else actionPlane->exitEasyMode();
		break;
	case SuperStrong:	_superStrong = !_superStrong; break;
	case Flying:
		if (GO::player->cheat(type)) // try to enable flying mode
			_flying = !_flying;
		break;
	case SuperJump:		_superJump = !_superJump; break;
	case MultiTeasures: _multiTreasures = !_multiTreasures; break;

	case IncResolution: WindowManager::setWindowScale(WindowManager::getWindowScale() + 0.5f); break;
	case DecResolution: WindowManager::setWindowScale(WindowManager::getWindowScale() - 0.5f); break;
	case DefaultResolution: WindowManager::setDefaultWindowScale(); break;

	default: break;
	}
}

int CheatsManager::getCheatType()
{
	size_t keysSize = keys.size();

	for (const auto& [type, cheatKey, msg] : cheatsKeys) {
		size_t cheatKeySize = strlen(cheatKey);

		if (keysSize == cheatKeySize && !strncmp(keys.data(), cheatKey, cheatKeySize)) {
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
			actionPlane->writeMessage(temp[0] ? temp : msg);

			return type;
		}
	}

	return Types::None;
}

void CheatsManager::addPowerup(int8_t powerupType)
{
	WwdObject obj;
	obj.smarts = 30000; // 30 seconds
	Item* item = Item::getItem(obj, powerupType);
	item->position = GO::getPlayerPosition();
	GO::player->collectItem(item);
	item->playItemSound();
	delete item;
}
