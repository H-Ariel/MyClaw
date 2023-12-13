#pragma once

#include "BaseEngine.h"
#include "LevelHUD.h"


struct ClawLevelEngineFields
{
	ClawLevelEngineFields(int levelNumber);
	~ClawLevelEngineFields();

	shared_ptr<WapWorld> _wwd;
	D2D1_POINT_2F* _mainPlanePosition;
	LevelHUD* _hud;
	ColorF _saveBgColor;
	float _savePixelSize;
	const int _levelNumber;
};

class ClawLevelEngine : public BaseEngine
{
public:
	ClawLevelEngine(int levelNumber, int checkpoint);
	ClawLevelEngine(shared_ptr<ClawLevelEngineFields> fields);

	void Logic(uint32_t elapsedTime) override;
	void Draw() override;

	void OnKeyUp(int key) override;
	void OnKeyDown(int key) override;


private:
	enum class States : int8_t {
		Play,	// normal gameplay
		Fall,	// CC falls out the window
		Close,	// close the screen
		Open	// open the screen
	};

	shared_ptr<ClawLevelEngineFields> _fields; // save fields for easy access after ingame-menu
	float _holeRadius; // the radius of the hole that remains until closed
	bool _deathAniWait; // waiting for disqualification animation to finish
	bool _playDeathSound;
	States _state;
};
