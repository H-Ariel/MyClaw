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
		// NOTE: don't use OpenLevel and LoadCheckpoint directly!
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

	HierarchicalMenu(vector<HierarchicalMenu> subMenus)
		: HierarchicalMenu("", "", Nop, subMenus) {}
	HierarchicalMenu(const string& pcxPath)
		: HierarchicalMenu(pcxPath, "", Nop, {}) {}
	HierarchicalMenu(const string& pcxPath, uint8_t cmd, vector<HierarchicalMenu> subMenus)
		: HierarchicalMenu(pcxPath, "", cmd, subMenus) {}
	HierarchicalMenu(const string& pcxPath, const string& markedPcxPath, vector<HierarchicalMenu> subMenus)
		: HierarchicalMenu(pcxPath, markedPcxPath, MenuIn, subMenus) {}
	HierarchicalMenu(const string& pcxPath, const string& markedPcxPath, uint8_t cmd, vector<HierarchicalMenu> subMenus = {})
		: pcxPath(pcxPath), markedPcxPath(markedPcxPath), subMenus(subMenus), cmd(cmd) {}

	vector<HierarchicalMenu> subMenus;
	string pcxPath;
	string markedPcxPath;
	uint8_t cmd;

	static HierarchicalMenu MainMenu, InGameMenu;
	static HierarchicalMenu SelectLevelMenu, SelectCheckpoint;
};
