#pragma once

#include "Framework/Framework.h"

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

		LoadCheckpoint_Start = LoadCheckpoint | (0x10),
		LoadCheckpoint_1 = LoadCheckpoint | (0x20),
		LoadCheckpoint_2 = LoadCheckpoint | (0x30),

		// ingame menu commands:
		BackToGame = OpenLevel + 1,
		EndLife,
		EndGame
	};

	HierarchicalMenu(const string& pcxPath, uint8_t cmd, float xRatio, float yRatio, vector<HierarchicalMenu> subMenus = {})
		: pcxPath(pcxPath), subMenus(subMenus), xRatio(xRatio), yRatio(yRatio), cmd(cmd) {}

	vector<HierarchicalMenu> subMenus;
	string pcxPath;
	float xRatio, yRatio;
	uint8_t cmd;

	static HierarchicalMenu MainMenu, InGameMenu;
	static HierarchicalMenu SelectLevelMenu, SelectCheckpoint;
};
