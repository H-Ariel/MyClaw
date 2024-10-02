#pragma once

#include "Framework/Buffer.h"


struct MenuData
{
	MenuData(shared_ptr<Buffer> menuFileStream);

	string name;
	vector<string> images;
	vector<MenuData> subMenus;
	uint16_t magicNumber;
};
