#pragma once

#include "BaseEngine.h"
#include "LevelHUD.h"


class CheatsManager;

struct ClawLevelEngineFields
{
	ClawLevelEngineFields(int levelNumber);
	~ClawLevelEngineFields();

	shared_ptr<WapWorld> _wwd;
	D2D1_POINT_2F* _mainPlanePosition;
	LevelHUD* _hud;
	CheatsManager* _cheatsManager;
	ColorF _saveBgColor;
	float _savePixelSize;
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

	static void playerEnterWarp(D2D1_POINT_2F destination, bool isBossWarp, float bossWarpX);

private:
	void init(); // call this in each constructor

	enum class States : int8_t {
		Play,	// normal gameplay
		Fall,	// CC falls out the window
		Close,	// close screen
		Open,	// open screen
		GameOver // game over screen
	};

	shared_ptr<ClawLevelEngineFields> _fields; // save fields for easy access after ingame-menu
	
	float _holeRadius; // the radius of the hole that remains until closed
	bool _deathAniWait; // waiting for disqualification animation to finish
	bool _playDeathSound;
	
	D2D1_POINT_2F _wrapDestination;
	float _wrapCoverTop ; // used in wrap transition animation
	bool _wrapAniWait; // waiting for warp transition animation to finish
	float _bossWarpX;
	bool _isBossWarp;

	int _gameOverTimeCounter; // used to delay game over screen

	States _state;
};
