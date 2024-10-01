#pragma once

#include "Framework/Framework.h"
#include "Framework/DynamicArray.hpp"


class CheatsManager
{
public:
	enum Types
	{
		None,
		FireSword,
		IceSword,
		LightningSword,
		Catnip,
		Invisibility,
		Invincibility,
		FillHealth,
		FillPistol,
		FillMagic,
		FillDynamite,
		FillLife,
		FinishLevel,
		GodMode,
		EasyMode,
		SuperStrong,
		Flying,
		SuperJump,
		BgMscSpeedUp,
		BgMscSlowDown,
		BgMscNormal,
		MultiTeasures,
		IncResolution,
		DecResolution,
		DefaultResolution
	};

	CheatsManager();

	void addKey(int key); // add key to the cheat sequence. if the sequence is correct, cheat will be activated

	bool isGodMode() const { return _god; }
	bool isEasyMode() const { return _easy; }
	bool isSuperStrong() const { return _superStrong; }
	bool isFlying() const { return _flying; }
	bool isSuperJump() const { return _superJump; }
	bool isMultiTreasures() const { return _multiTreasures; }

private:
	int getCheatType();
	void addPowerup(int8_t powerupType);

	static const tuple<int, const char*, const char*> cheatsKeys[]; // cheat keys list
	vector<char> keys; // keys pressed
	bool _god, _easy, _superStrong, _flying, _superJump, _multiTreasures;
};
