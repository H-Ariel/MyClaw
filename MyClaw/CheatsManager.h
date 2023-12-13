#pragma once

#include "framework.h"


class CheatsManager
{
public:
	static void addKey(int key);

private:
	static int getCheatType();

	static void fireSword();
	static void iceSword();
	static void catnip();

	static void addPowerup(int8_t powerupType);


	static vector<int> keys;
	static map<int, vector<int>> cheatKeys;
};
