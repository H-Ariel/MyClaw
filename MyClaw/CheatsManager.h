#pragma once

#include "framework.h"


class CheatsManager
{
public:
	CheatsManager();

	void addKey(int key);

private:
	int getCheatType();
	void addPowerup(int8_t powerupType);

	vector<char> keys; // keys pressed
	const vector<tuple<int, const char*, const char*>> cheatKeys;
};
