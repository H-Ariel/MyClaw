#include "HierarchicalMenu.h"


HierarchicalMenu HierarchicalMenu::MainMenu("", MenuIn, 0, 0, {
	HierarchicalMenu(MAIN_MENU_ROOT "014.PCX", Nop, 0, -0.15f),
	HierarchicalMenu(MAIN_MENU_ROOT "001.PCX", MenuIn, 0, -0.05f, {
		HierarchicalMenu(SINGLEPLAYER_ROOT "014.PCX", Nop, 0, -0.15f),
		HierarchicalMenu(SINGLEPLAYER_ROOT "001.PCX", SelectLevel, 0, -0.05f),
		HierarchicalMenu(SINGLEPLAYER_ROOT "003.PCX", SelectLevel, 0, 0.04f),
		HierarchicalMenu(SINGLEPLAYER_ROOT "005.PCX", NotImpleted, 0, 0.13f),
		HierarchicalMenu(SINGLEPLAYER_ROOT "007.PCX", MenuIn, 0, 0.22f, {
			HierarchicalMenu(SINGLEPLAYER_ROOT "SAVE/003.PCX", Nop, 0, 0),
			HierarchicalMenu(SINGLEPLAYER_ROOT "SAVE/001.PCX", MenuOut, 0, 0.22f),
		}),
		HierarchicalMenu(SINGLEPLAYER_ROOT "009.PCX", NotImpleted, 0, 0.31f),
		HierarchicalMenu(SINGLEPLAYER_ROOT "012.PCX", MenuOut, 0, 0.4f),
	}),
	HierarchicalMenu(MAIN_MENU_ROOT "003.PCX", NotImpleted, 0, 0.03f),
	HierarchicalMenu(MAIN_MENU_ROOT "005.PCX", NotImpleted, 0, 0.11f),
	HierarchicalMenu(MAIN_MENU_ROOT "008.PCX", NotImpleted, 0, 0.19f),
	HierarchicalMenu(MAIN_MENU_ROOT "015.PCX", Credits, 0, 0.27f),
	HierarchicalMenu(MAIN_MENU_ROOT "010.PCX", Help, 0, 0.35f),
	HierarchicalMenu(MAIN_MENU_ROOT "012.PCX", MenuIn, 0, 0.43f, {
		HierarchicalMenu(MAIN_MENU_ROOT "QUIT/005.PCX", Nop, 0, 0),
		HierarchicalMenu(MAIN_MENU_ROOT "QUIT/001.PCX", ExitApp, 0, 0.15f),
		HierarchicalMenu(MAIN_MENU_ROOT "QUIT/003.PCX", MenuOut, 0, 0.22f),
	})
});

HierarchicalMenu HierarchicalMenu::InGameMenu("", MenuIn, 0, 0, {
	HierarchicalMenu(INGAME_MENU_ROOT "001.PID", BackToGame, 0, -0.16f),
	HierarchicalMenu(INGAME_MENU_ROOT "003.PID", MenuIn, 0, -0.08f, {
		HierarchicalMenu(INGAME_MENU_ROOT "CLAWICIDE/005.PID", Nop, 0, -0.10f),
		HierarchicalMenu(INGAME_MENU_ROOT "CLAWICIDE/001.PID", EndLife, 0, 0.05f),
		HierarchicalMenu(INGAME_MENU_ROOT "CLAWICIDE/003.PID", MenuOut, 0, 0.125f),
	}),
	HierarchicalMenu(INGAME_MENU_ROOT "005.PID", NotImpleted, 0, 0),
	HierarchicalMenu(INGAME_MENU_ROOT "007.PID", Help, 0, 0.08f),
	HierarchicalMenu(INGAME_MENU_ROOT "009.PID", MenuIn, 0, 0.16f, {
		HierarchicalMenu(INGAME_MENU_ROOT "ENDGAME/005.PID", Nop, 0, -0.10f),
		HierarchicalMenu(INGAME_MENU_ROOT "ENDGAME/001.PID", EndGame, 0, 0.05f),
		HierarchicalMenu(INGAME_MENU_ROOT "ENDGAME/003.PID", MenuOut, 0, 0.125f),
	}),
});

HierarchicalMenu HierarchicalMenu::SelectLevelMenu("", Nop, 0, 0, {
	HierarchicalMenu("<TITLE>", Nop, 0, -0.15f),
	HierarchicalMenu(SINGLEPLAYER_ROOT "LEVELS/001.PCX", OpenLevel | (0x10), -0.2f, -0.05f),
	HierarchicalMenu(SINGLEPLAYER_ROOT "LEVELS/004.PCX", OpenLevel | (0x20), -0.2f, 0.01f),
	HierarchicalMenu(SINGLEPLAYER_ROOT "LEVELS/007.PCX", OpenLevel | (0x30), -0.2f, 0.07f),
	HierarchicalMenu(SINGLEPLAYER_ROOT "LEVELS/010.PCX", OpenLevel | (0x40), -0.2f, 0.13f),
	HierarchicalMenu(SINGLEPLAYER_ROOT "LEVELS/013.PCX", OpenLevel | (0x50), -0.2f, 0.19f),
	HierarchicalMenu(SINGLEPLAYER_ROOT "LEVELS/016.PCX", OpenLevel | (0x60), -0.2f, 0.25f),
	HierarchicalMenu(SINGLEPLAYER_ROOT "LEVELS/019.PCX", OpenLevel | (0x70), -0.2f, 0.31f),
	HierarchicalMenu(SINGLEPLAYER_ROOT "LEVELS/022.PCX", OpenLevel | (0x80), 0.2f, -0.05f),
	HierarchicalMenu(SINGLEPLAYER_ROOT "LEVELS/025.PCX", OpenLevel | (0x90), 0.2f, 0.01f),
	HierarchicalMenu(SINGLEPLAYER_ROOT "LEVELS/028.PCX", OpenLevel | (0xA0), 0.2f, 0.07f),
	HierarchicalMenu(SINGLEPLAYER_ROOT "LEVELS/031.PCX", OpenLevel | (0xB0), 0.2f, 0.13f),
	HierarchicalMenu(SINGLEPLAYER_ROOT "LEVELS/034.PCX", OpenLevel | (0xC0), 0.2f, 0.19f),
	HierarchicalMenu(SINGLEPLAYER_ROOT "LEVELS/037.PCX", OpenLevel | (0xD0), 0.2f, 0.25f),
	HierarchicalMenu(SINGLEPLAYER_ROOT "LEVELS/040.PCX", OpenLevel | (0xE0), 0.2f, 0.31f),
	HierarchicalMenu(SINGLEPLAYER_ROOT "LEVELS/043.PCX", MenuOut, 0, 0.4f),
});

HierarchicalMenu HierarchicalMenu::SelectCheckpoint("", Nop, 0, 0, {
	HierarchicalMenu("<TITLE>", Nop, 0, -0.16f),
	HierarchicalMenu(LOAD_CHECKPOINT_ROOT "001.PCX", LoadCheckpoint_Start, 0, -0.08f),
	HierarchicalMenu(LOAD_CHECKPOINT_ROOT "003.PCX", LoadCheckpoint_1, 0, 0),
	HierarchicalMenu(LOAD_CHECKPOINT_ROOT "006.PCX", LoadCheckpoint_2, 0, 0.08f),
	HierarchicalMenu(LOAD_CHECKPOINT_ROOT "009.PCX", MenuOut, 0, 0.16f),
});
