#include "HierarchicalMenu.h"

#define OPTIONS_ROOT	MAIN_MENU_ROOT "OPTIONS/"

// TODO: add Slider-Buttons in HierarchicalMenu

HierarchicalMenu HierarchicalMenu::MainMenu({
	// title
	HierarchicalMenu(MAIN_MENU_ROOT "014.PCX"),
	// single player
	HierarchicalMenu(MAIN_MENU_ROOT "001.PCX", MAIN_MENU_ROOT "002.PCX", {
		HierarchicalMenu(SINGLEPLAYER_ROOT "014.PCX"),
		HierarchicalMenu(SINGLEPLAYER_ROOT "001.PCX", SINGLEPLAYER_ROOT "002.PCX", SelectLevel),
		HierarchicalMenu(SINGLEPLAYER_ROOT "003.PCX", SINGLEPLAYER_ROOT "004.PCX", SelectLevel),
		HierarchicalMenu(SINGLEPLAYER_ROOT "005.PCX", SINGLEPLAYER_ROOT "006.PCX", NotImpleted),
		HierarchicalMenu(SINGLEPLAYER_ROOT "007.PCX", SINGLEPLAYER_ROOT "008.PCX", MenuIn, {
			HierarchicalMenu(SINGLEPLAYER_ROOT "SAVE/003.PCX"),
			HierarchicalMenu(SINGLEPLAYER_ROOT "SAVE/001.PCX", SINGLEPLAYER_ROOT "SAVE/002.PCX", MenuOut),
		}),
		HierarchicalMenu(SINGLEPLAYER_ROOT "009.PCX", SINGLEPLAYER_ROOT "010.PCX", NotImpleted),
		HierarchicalMenu(SINGLEPLAYER_ROOT "012.PCX", SINGLEPLAYER_ROOT "013.PCX", MenuOut),
	}),
	// multi player
	HierarchicalMenu(MAIN_MENU_ROOT "003.PCX", MAIN_MENU_ROOT "004.PCX", NotImpleted),
	// replay movies
	HierarchicalMenu(MAIN_MENU_ROOT "005.PCX", MAIN_MENU_ROOT "006.PCX", NotImpleted),
	// options
	HierarchicalMenu(MAIN_MENU_ROOT "008.PCX", MAIN_MENU_ROOT "009.PCX", MenuIn, {
		// title
		HierarchicalMenu(OPTIONS_ROOT "014.PCX"),
		// edit players
		HierarchicalMenu(OPTIONS_ROOT "001.PCX", OPTIONS_ROOT "002.PCX", NotImpleted),
		// controls
		HierarchicalMenu(OPTIONS_ROOT "005.PCX", OPTIONS_ROOT "006.PCX", NotImpleted),
		// display
		HierarchicalMenu(OPTIONS_ROOT "010.PCX", OPTIONS_ROOT "011.PCX", MenuIn, {
			// title
			HierarchicalMenu(OPTIONS_ROOT "DISPLAY/021.PCX"),
			// details
			HierarchicalMenu(OPTIONS_ROOT "DISPLAY/005.PCX", OPTIONS_ROOT "DISPLAY/007.PCX",
							 OPTIONS_ROOT "DISPLAY/006.PCX", OPTIONS_ROOT "DISPLAY/008.PCX", Details),
			// front layer
			HierarchicalMenu(OPTIONS_ROOT "DISPLAY/009.PCX", OPTIONS_ROOT "DISPLAY/011.PCX",
							 OPTIONS_ROOT "DISPLAY/010.PCX", OPTIONS_ROOT "DISPLAY/012.PCX", FrontLayer),
			// game play area
			HierarchicalMenu(OPTIONS_ROOT "DISPLAY/013.PCX", OPTIONS_ROOT "DISPLAY/014.PCX", NotImpleted),
			// movies
			HierarchicalMenu(OPTIONS_ROOT "DISPLAY/015.PCX", OPTIONS_ROOT "DISPLAY/017.PCX",
							 OPTIONS_ROOT "DISPLAY/016.PCX", OPTIONS_ROOT "DISPLAY/018.PCX", Movies),
			// back
			HierarchicalMenu(OPTIONS_ROOT "DISPLAY/019.PCX", OPTIONS_ROOT "DISPLAY/020.PCX", MenuOut)
		}),
		// audio
		HierarchicalMenu(OPTIONS_ROOT "007.PCX", OPTIONS_ROOT "008.PCX", MenuIn, {
			// title
			HierarchicalMenu(OPTIONS_ROOT "AUDIO/019.PCX"),
			// sound
			HierarchicalMenu(OPTIONS_ROOT "AUDIO/001.PCX", OPTIONS_ROOT "AUDIO/003.PCX",
							 OPTIONS_ROOT "AUDIO/002.PCX", OPTIONS_ROOT "AUDIO/004.PCX", Sound),
			// voice
			HierarchicalMenu(OPTIONS_ROOT "AUDIO/005.PCX", OPTIONS_ROOT "AUDIO/007.PCX",
							 OPTIONS_ROOT "AUDIO/006.PCX", OPTIONS_ROOT "AUDIO/008.PCX", Voice),
			// ambient
			HierarchicalMenu(OPTIONS_ROOT "AUDIO/009.PCX", OPTIONS_ROOT "AUDIO/011.PCX",
							 OPTIONS_ROOT "AUDIO/010.PCX", OPTIONS_ROOT "AUDIO/012.PCX", Ambient),
			// music
			HierarchicalMenu(OPTIONS_ROOT "AUDIO/013.PCX", OPTIONS_ROOT "AUDIO/015.PCX",
							 OPTIONS_ROOT "AUDIO/014.PCX", OPTIONS_ROOT "AUDIO/016.PCX", Music),
			// back
			HierarchicalMenu(OPTIONS_ROOT "AUDIO/017.PCX", OPTIONS_ROOT "AUDIO/018.PCX", MenuOut)
		}),
		// back
		HierarchicalMenu(OPTIONS_ROOT "012.PCX", OPTIONS_ROOT "013.PCX", MenuOut),
	}),
	// credits
	HierarchicalMenu(MAIN_MENU_ROOT "015.PCX", MAIN_MENU_ROOT "016.PCX", Credits),
	// help
	HierarchicalMenu(MAIN_MENU_ROOT "010.PCX", MAIN_MENU_ROOT "011.PCX", Help),
	// quit
	HierarchicalMenu(MAIN_MENU_ROOT "012.PCX", MAIN_MENU_ROOT "013.PCX", MenuIn, {
		HierarchicalMenu(MAIN_MENU_ROOT "QUIT/005.PCX"),
		HierarchicalMenu(MAIN_MENU_ROOT "QUIT/001.PCX", MAIN_MENU_ROOT "QUIT/002.PCX", ExitApp),
		HierarchicalMenu(MAIN_MENU_ROOT "QUIT/003.PCX", MAIN_MENU_ROOT "QUIT/004.PCX", MenuOut),
	})
});

HierarchicalMenu HierarchicalMenu::InGameMenu({
	HierarchicalMenu(INGAME_MENU_ROOT "001.PID", INGAME_MENU_ROOT "002.PID", BackToGame),
	HierarchicalMenu(INGAME_MENU_ROOT "003.PID", INGAME_MENU_ROOT "004.PID", MenuIn, {
		HierarchicalMenu(INGAME_MENU_ROOT "CLAWICIDE/005.PID"),
		HierarchicalMenu(INGAME_MENU_ROOT "CLAWICIDE/001.PID", INGAME_MENU_ROOT "CLAWICIDE/002.PID", EndLife),
		HierarchicalMenu(INGAME_MENU_ROOT "CLAWICIDE/003.PID", INGAME_MENU_ROOT "CLAWICIDE/004.PID", MenuOut),
	}),
	HierarchicalMenu(INGAME_MENU_ROOT "005.PID", INGAME_MENU_ROOT "006.PID", NotImpleted),
	HierarchicalMenu(INGAME_MENU_ROOT "007.PID", INGAME_MENU_ROOT "008.PID", Help),
	HierarchicalMenu(INGAME_MENU_ROOT "009.PID", INGAME_MENU_ROOT "010.PID", MenuIn, {
		HierarchicalMenu(INGAME_MENU_ROOT "ENDGAME/005.PID"),
		HierarchicalMenu(INGAME_MENU_ROOT "ENDGAME/001.PID", INGAME_MENU_ROOT "ENDGAME/002.PID", EndGame),
		HierarchicalMenu(INGAME_MENU_ROOT "ENDGAME/003.PID", INGAME_MENU_ROOT "ENDGAME/004.PID", MenuOut),
	}),
});

HierarchicalMenu HierarchicalMenu::SelectLevelMenu({
	HierarchicalMenu("<TITLE>"),
	HierarchicalMenu(SINGLEPLAYER_ROOT "LEVELS/001.PCX", SINGLEPLAYER_ROOT "LEVELS/002.PCX", OpenLevel | (0x10)),
	HierarchicalMenu(SINGLEPLAYER_ROOT "LEVELS/004.PCX", SINGLEPLAYER_ROOT "LEVELS/005.PCX", OpenLevel | (0x20)),
	HierarchicalMenu(SINGLEPLAYER_ROOT "LEVELS/007.PCX", SINGLEPLAYER_ROOT "LEVELS/008.PCX", OpenLevel | (0x30)),
	HierarchicalMenu(SINGLEPLAYER_ROOT "LEVELS/010.PCX", SINGLEPLAYER_ROOT "LEVELS/011.PCX", OpenLevel | (0x40)),
	HierarchicalMenu(SINGLEPLAYER_ROOT "LEVELS/013.PCX", SINGLEPLAYER_ROOT "LEVELS/014.PCX", OpenLevel | (0x50)),
	HierarchicalMenu(SINGLEPLAYER_ROOT "LEVELS/016.PCX", SINGLEPLAYER_ROOT "LEVELS/017.PCX", OpenLevel | (0x60)),
	HierarchicalMenu(SINGLEPLAYER_ROOT "LEVELS/019.PCX", SINGLEPLAYER_ROOT "LEVELS/020.PCX", OpenLevel | (0x70)),
	HierarchicalMenu(SINGLEPLAYER_ROOT "LEVELS/022.PCX", SINGLEPLAYER_ROOT "LEVELS/023.PCX", OpenLevel | (0x80)),
	HierarchicalMenu(SINGLEPLAYER_ROOT "LEVELS/025.PCX", SINGLEPLAYER_ROOT "LEVELS/026.PCX", OpenLevel | (0x90)),
	HierarchicalMenu(SINGLEPLAYER_ROOT "LEVELS/028.PCX", SINGLEPLAYER_ROOT "LEVELS/029.PCX", OpenLevel | (0xA0)),
	HierarchicalMenu(SINGLEPLAYER_ROOT "LEVELS/031.PCX", SINGLEPLAYER_ROOT "LEVELS/032.PCX", OpenLevel | (0xB0)),
	HierarchicalMenu(SINGLEPLAYER_ROOT "LEVELS/034.PCX", SINGLEPLAYER_ROOT "LEVELS/035.PCX", OpenLevel | (0xC0)),
	HierarchicalMenu(SINGLEPLAYER_ROOT "LEVELS/037.PCX", SINGLEPLAYER_ROOT "LEVELS/038.PCX", OpenLevel | (0xD0)),
	HierarchicalMenu(SINGLEPLAYER_ROOT "LEVELS/040.PCX", SINGLEPLAYER_ROOT "LEVELS/041.PCX", OpenLevel | (0xE0)),
	HierarchicalMenu(SINGLEPLAYER_ROOT "LEVELS/043.PCX", SINGLEPLAYER_ROOT "LEVELS/044.PCX", MenuOut),
});

HierarchicalMenu HierarchicalMenu::SelectCheckpoint({
	HierarchicalMenu("<TITLE>"),
	HierarchicalMenu(LOAD_CHECKPOINT_ROOT "001.PCX", LOAD_CHECKPOINT_ROOT "002.PCX", LoadCheckpoint_Start),
	HierarchicalMenu(LOAD_CHECKPOINT_ROOT "003.PCX", LOAD_CHECKPOINT_ROOT "004.PCX", LoadCheckpoint_1),
	HierarchicalMenu(LOAD_CHECKPOINT_ROOT "006.PCX", LOAD_CHECKPOINT_ROOT "007.PCX", LoadCheckpoint_2),
	HierarchicalMenu(LOAD_CHECKPOINT_ROOT "009.PCX", LOAD_CHECKPOINT_ROOT "010.PCX", MenuOut),
});
