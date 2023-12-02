#include "HierarchicalMenu.h"


HierarchicalMenu HierarchicalMenu::MainMenu("", MenuIn, 0, 0, {
	HierarchicalMenu(MAIN_MENU_ROOT "014_TITLE.PCX", Nop, 0, -0.15f),
	HierarchicalMenu(MAIN_MENU_ROOT "001_SINGLEPLAYER.PCX", MenuIn, 0, -0.05f, {
		HierarchicalMenu(SINGLEPLAYER_ROOT "014_TITLE.PCX", Nop, 0, -0.15f),
		HierarchicalMenu(SINGLEPLAYER_ROOT "001_NEWGAME.PCX", SelectLevel, 0, -0.05f),
		HierarchicalMenu(SINGLEPLAYER_ROOT "003_LOADGAME.PCX", SelectLevel, 0, 0.04f),
		HierarchicalMenu(SINGLEPLAYER_ROOT "005_LOADCUSTOM.PCX", NotImpleted, 0, 0.13f),
		HierarchicalMenu(SINGLEPLAYER_ROOT "007_SAVEGAME.PCX", MenuIn, 0, 0.22f, {
			HierarchicalMenu(SINGLEPLAYER_ROOT "SAVE/003_EXPLANATION.PCX", Nop, 0, 0),
			HierarchicalMenu(SINGLEPLAYER_ROOT "SAVE/001_PREVIOUSMENU.PCX", MenuOut, 0, 0.22f),
		}),
		HierarchicalMenu(SINGLEPLAYER_ROOT "009_UPLOADSCORES.PCX", NotImpleted, 0, 0.31f),
		HierarchicalMenu(SINGLEPLAYER_ROOT "012_PREVIOUSMENU.PCX", MenuOut, 0, 0.4f),
	}),
	HierarchicalMenu(MAIN_MENU_ROOT "003_MULTIPLAYER.PCX", NotImpleted, 0, 0.03f),
	HierarchicalMenu(MAIN_MENU_ROOT "005_REPLAYMOVIES.PCX", NotImpleted, 0, 0.11f),
	HierarchicalMenu(MAIN_MENU_ROOT "008_OPTIONS.PCX", NotImpleted, 0, 0.19f),
	HierarchicalMenu(MAIN_MENU_ROOT "015_CREDITS.PCX", Credits, 0, 0.27f),
	HierarchicalMenu(MAIN_MENU_ROOT "010_HELP.PCX", Help, 0, 0.35f),
	HierarchicalMenu(MAIN_MENU_ROOT "012_QUIT.PCX", MenuIn, 0, 0.43f, {
		HierarchicalMenu(MAIN_MENU_ROOT "QUIT/005_AREYOUSURE.PCX", Nop, 0, 0),
		HierarchicalMenu(MAIN_MENU_ROOT "QUIT/001_YES.PCX", ExitApp, 0, 0.15f),
		HierarchicalMenu(MAIN_MENU_ROOT "QUIT/003_NO.PCX", MenuOut, 0, 0.22f),
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
	HierarchicalMenu(SINGLEPLAYER_ROOT "LEVELS/001_LEVELONE.PCX",		OpenLevel_1, -0.2f, -0.05f),
	HierarchicalMenu(SINGLEPLAYER_ROOT "LEVELS/004_LEVELTWO.PCX",		OpenLevel_2, -0.2f, 0.01f),
	HierarchicalMenu(SINGLEPLAYER_ROOT "LEVELS/007_LEVELTHREE.PCX",		OpenLevel_3, -0.2f, 0.07f),
	HierarchicalMenu(SINGLEPLAYER_ROOT "LEVELS/010_LEVELFOUR.PCX",		OpenLevel_4, -0.2f, 0.13f),
	HierarchicalMenu(SINGLEPLAYER_ROOT "LEVELS/013_LEVELFIVE.PCX",		OpenLevel_5, -0.2f, 0.19f),
	HierarchicalMenu(SINGLEPLAYER_ROOT "LEVELS/016_LEVELSIX.PCX",		OpenLevel_6, -0.2f, 0.25f),
	HierarchicalMenu(SINGLEPLAYER_ROOT "LEVELS/019_LEVELSEVEN.PCX",		OpenLevel_7, -0.2f, 0.31f),
	HierarchicalMenu(SINGLEPLAYER_ROOT "LEVELS/022_LEVELEIGHT.PCX",		OpenLevel_8, 0.2f, -0.05f),
	HierarchicalMenu(SINGLEPLAYER_ROOT "LEVELS/025_LEVELNINE.PCX",		OpenLevel_9, 0.2f, 0.01f),
	HierarchicalMenu(SINGLEPLAYER_ROOT "LEVELS/028_LEVELTEN.PCX",		OpenLevel_10, 0.2f, 0.07f),
	HierarchicalMenu(SINGLEPLAYER_ROOT "LEVELS/031_LEVELELEVEN.PCX",	OpenLevel_11, 0.2f, 0.13f),
	HierarchicalMenu(SINGLEPLAYER_ROOT "LEVELS/034_LEVELTWELVE.PCX",	OpenLevel_12, 0.2f, 0.19f),
	HierarchicalMenu(SINGLEPLAYER_ROOT "LEVELS/037_LEVELTHIRTEEN.PCX",	OpenLevel_13, 0.2f, 0.25f),
	HierarchicalMenu(SINGLEPLAYER_ROOT "LEVELS/040_LEVELFOURTEEN.PCX",	OpenLevel_14, 0.2f, 0.31f),
	HierarchicalMenu(SINGLEPLAYER_ROOT "LEVELS/043_BACK.PCX", MenuOut, 0, 0.4f),
});

HierarchicalMenu HierarchicalMenu::SelectCheckpoint("", Nop, 0, 0, {
	HierarchicalMenu("<TITLE>", Nop, 0, -0.16f),
	HierarchicalMenu(LOAD_CHECKPOINT_ROOT "001_START.PCX", LoadCheckpoint_Start, 0, -0.08f),
	HierarchicalMenu(LOAD_CHECKPOINT_ROOT "003_SAVEONE.PCX", LoadCheckpoint_1, 0, 0),
	HierarchicalMenu(LOAD_CHECKPOINT_ROOT "006_SAVETWO.PCX", LoadCheckpoint_2, 0, 0.08f),
	HierarchicalMenu(LOAD_CHECKPOINT_ROOT "009_PREVIOUSMENU.PCX", MenuOut, 0, 0.16f),
});
