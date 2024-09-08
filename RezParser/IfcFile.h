#pragma once

#include "Framework/BufferReader.h"


struct MenuData
{
	MenuData(shared_ptr<BufferReader> menuFileStream);

	string name;
	vector<string> images;
	vector<MenuData> subMenus;
	uint16_t magicNumber;
};
