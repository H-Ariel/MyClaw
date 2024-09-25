#pragma once

#include "Framework/Framework.h"

#define MAIN_MENU_ROOT			"STATES/MENU/IMAGES/MAIN/"
#define SINGLEPLAYER_ROOT		MAIN_MENU_ROOT "SINGLEPLAYER/"
#define LOAD_CHECKPOINT_ROOT	SINGLEPLAYER_ROOT "LOAD/CHECKPOINTS/"
#define OPTIONS_ROOT			MAIN_MENU_ROOT "OPTIONS/"


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
		Credits,
		// NOTE: don't use OpenLevel and LoadCheckpoint directly!
		OpenLevel, // `OpenLevel | (<lvlNo> << 4)`
		LoadCheckpoint, // `LoadCheckpoint | (<chkPntNo> << 4)`
		LoadCheckpoint_Start = LoadCheckpoint | (0x10),
		LoadCheckpoint_1 = LoadCheckpoint | (0x20),
		LoadCheckpoint_2 = LoadCheckpoint | (0x30),

		// ingame menu commands:
		BackToGame = (OpenLevel | (14 << 4)) + 1, // pass commands to avoid conflicts
		EndLife,
		EndGame,

		// display settings:
		Details,    // High / Low
		FrontLayer, // On / Off
		Area,       // Slider (0-9) [0 is Off, else is On]
		Movies,     // Stretched / Interlaced
		// audio settings:
		Sound,      // Slider (0-9) [0 is Off, else is On]
		Voice,      // On / Off
		Ambient,    // On / Off
		Music,      // Slider (0-9) [0 is Off, else is On]
	};

	HierarchicalMenu(vector<HierarchicalMenu> subMenus)
		: HierarchicalMenu("", "", Nop, subMenus) {}
	HierarchicalMenu(const string& imagePath)
		: HierarchicalMenu(imagePath, "", Nop, {}) {}
	HierarchicalMenu(const string& imagePath, uint8_t cmd, vector<HierarchicalMenu> subMenus)
		: HierarchicalMenu(imagePath, "", cmd, subMenus) {}
	HierarchicalMenu(const string& imagePath, const string& markedImagePath, vector<HierarchicalMenu> subMenus)
		: HierarchicalMenu(imagePath, markedImagePath, MenuIn, subMenus) {}
	HierarchicalMenu(const string& pcxPath1, const string& markedPcxPath1, const string& toggleStateImage, const string& markedToggleStateImage, uint8_t cmd)
		: imagePath(pcxPath1), markedImagePath(markedPcxPath1), toggleStateImage(toggleStateImage), markedToggleStateImage(markedToggleStateImage), cmd(cmd) {}
	HierarchicalMenu(const string& imagePath, const string& markedImagePath, uint8_t cmd, vector<HierarchicalMenu> subMenus = {})
		: imagePath(imagePath), markedImagePath(markedImagePath), subMenus(subMenus), cmd(cmd) {}


	vector<HierarchicalMenu> subMenus;
	string imagePath, markedImagePath;
	string toggleStateImage, markedToggleStateImage; // if this is a two-state button (like "Details: [HIGH|LOW]")
	uint8_t cmd;
	
	static HierarchicalMenu OptionsMenu, HelpScreen;
	static HierarchicalMenu MainMenu, InGameMenu;
	static HierarchicalMenu SelectLevelMenu, SelectCheckpoint;
};
