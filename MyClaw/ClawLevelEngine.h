#pragma once

#include "GameEngine/BaseEngine.h"
#include "LevelHUD.h"


class LevelPlane;

struct ClawLevelEngineFields
{
	ClawLevelEngineFields(int levelNumber);
	~ClawLevelEngineFields();

	shared_ptr<WapWwd> _wwd;
	vector<shared_ptr<LevelPlane>> _planes;
	D2D1_POINT_2F* _mainPlanePosition;
	LevelHUD* _hud;
	ColorF _saveBgColor;
	float _saveWindowScale;
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

	void OnResize() override;

private:
	void init(); // call this in each constructor

	enum class States : int8_t {
		Play,
		DeathFall,
		DeathClose,
		DeathOpen,
		WrapClose,
		WrapOpen,
		GameOver
	};

	shared_ptr<ClawLevelEngineFields> _fields; // save fields for easy access after ingame-menu
	
	float _holeRadius; // the radius of the hole that remains until closed
	
	D2D1_POINT_2F _wrapDestination;
	float _wrapCoverTop ; // used in wrap transition animation
	float _bossWarpX;
	bool _isBossWarp;

	int _gameOverTimeCounter; // used to delay game over screen

	States _state;
};
