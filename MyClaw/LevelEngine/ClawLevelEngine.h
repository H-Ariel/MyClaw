#pragma once

#include "GameEngine/BaseEngine.h"
#include "LevelHUD.h"
#include "InputController.h"


class LevelPlane;
class ActionPlane;
class ClawLevelEngineState;


class ClawLevelEngine : public BaseEngine
{
public:
	static shared_ptr<ClawLevelEngine> create(int levelNumber, int checkpoint);

	~ClawLevelEngine();

	void Logic(uint32_t elapsedTime) override;
	void Draw() override;

	void OnKeyUp(int key) override;
	void OnKeyDown(int key) override;

	void OnResize() override;

	// the warp player will be teleported to
	void playerEnterWrap(Warp* DestinationWarp);

	ClawLevelEngineState* getState() const { return _state; }
	void switchState(ClawLevelEngineState* newState);

	static shared_ptr<ClawLevelEngine> getInstance() { return _instance; }
	static void destrotInstance() { _instance = nullptr; }


private:
	ClawLevelEngine(int levelNumber, int checkpoint); // private c'tor, because `create` method sets `_instance`

	float getMaximalHoleRadius() const; // used for DeathClose and DeathOpen

	static shared_ptr<ClawLevelEngine> _instance;

	shared_ptr<WapWwd> _wwd;
	vector<shared_ptr<LevelPlane>> _planes;
	InputController* _inputController;
	ActionPlane* actionPlane;
	LevelHUD* _hud;
	ColorF _saveBgColor;
	float _saveWindowScale;
	
	ClawLevelEngineState* _state, *_nextState;


	friend class PlayState;
	friend class DeathFallState;
	friend class DeathCloseState;
	friend class DeathOpenState;
	friend class WrapCloseState;
	friend class WrapOpenState;
	friend class GameOverState;
};
