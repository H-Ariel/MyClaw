#pragma once

#include "Framework/Framework.h"


#define type_Treasure_Skull		Item::Treasure_Skull_Blue
#define type_Treasure_Crowns	Item::Treasure_Crowns_Green
#define type_Treasure_Geckos	Item::Treasure_Geckos_Red
#define type_Treasure_Scepters	Item::Treasure_Scepters_Red
#define type_Treasure_Crosses	Item::Treasure_Crosses_Blue
#define type_Treasure_Chalices	Item::Treasure_Chalices_Green
#define type_Treasure_Rings		Item::Treasure_Rings_Purple
#define type_Treasure_Goldbars	Item::Treasure_Goldbars
#define type_Treasure_Coins		Item::Treasure_Coins


inline string getBGImgPath(int img)
{
	char path[30];
	sprintf(path, "STATES/BOOTY/SCREENS/%03d.PCX", img);
	return path;
}
