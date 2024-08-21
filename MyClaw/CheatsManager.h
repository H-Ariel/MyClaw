#pragma once

#include "Framework/Framework.h"


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
		BgMscSpeedUp,
		BgMscSlowDown,
		BgMscNormal,
	};

	CheatsManager();

	void addKey(int key);

private:
	int getCheatType();
	void addPowerup(int8_t powerupType);

	vector<char> keys; // keys pressed
	const vector<tuple<int, const char*, const char*>> cheatKeys;
};
