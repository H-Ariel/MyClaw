#pragma once

#include "../framework.h"

#define MAIN_MENU_ROOT			"STATES/MENU/IMAGES/MAIN/"
#define SINGLEPLAYER_ROOT		MAIN_MENU_ROOT "SINGLEPLAYER/"
#define LOAD_CHECKPOINT_ROOT	SINGLEPLAYER_ROOT "LOAD/CHECKPOINTS/"
#define INGAME_MENU_ROOT		"GAME/IMAGES/INGAMEMENU/MAIN/"


struct HierarchicalMenu
{
	enum Commands : uint8_t
	{
		Nop,
		NotImpleted,
		MenuIn, // go to next sub-menu
		MenuOut, // back to previous menu
		SelectLevel,
		ExitApp,
		Help,
		Credits,
		OpenLevel, // `OpenLevel | (<lvlNo> << 4)`
		LoadCheckpoint, // `LoadCheckpoint | (<chkPntNo> << 4)`

		OpenLevel_1  = OpenLevel | (0x10),
		OpenLevel_2  = OpenLevel | (0x20),
		OpenLevel_3  = OpenLevel | (0x30),
		OpenLevel_4  = OpenLevel | (0x40),
		OpenLevel_5  = OpenLevel | (0x50),
		OpenLevel_6  = OpenLevel | (0x60),
		OpenLevel_7  = OpenLevel | (0x70),
		OpenLevel_8  = OpenLevel | (0x80),
		OpenLevel_9  = OpenLevel | (0x90),
		OpenLevel_10 = OpenLevel | (0xA0),
		OpenLevel_11 = OpenLevel | (0xB0),
		OpenLevel_12 = OpenLevel | (0xC0),
		OpenLevel_13 = OpenLevel | (0xD0),
		OpenLevel_14 = OpenLevel | (0xE0),

		LoadCheckpoint_Start = LoadCheckpoint | (0x10),
		LoadCheckpoint_1     = LoadCheckpoint | (0x20),
		LoadCheckpoint_2     = LoadCheckpoint | (0x30),

		// ingame menu commands:
		BackToGame = OpenLevel + 1,
		EndLife,
		EndGame
	};

	HierarchicalMenu(string pcxPath, uint8_t cmd, float xRatio, float yRatio, vector<HierarchicalMenu> subMenus = {})
		: pcxPath(pcxPath), subMenus(subMenus), xRatio(xRatio), yRatio(yRatio), cmd(cmd) {}

	vector<HierarchicalMenu> subMenus;
	string pcxPath;
	float xRatio, yRatio;
	uint8_t cmd;

	static HierarchicalMenu MainMenu, InGameMenu;
	static HierarchicalMenu SelectLevelMenu, SelectCheckpoint;
};
