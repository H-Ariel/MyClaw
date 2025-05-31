#pragma once

#include "GameEngine/BaseEngine.h"
#include "LevelHUD.h"


class LevelPlane;
class ActionPlane;
class ClawLevelEngineState;


class ClawLevelEngine : public BaseEngine
{
public:
	ClawLevelEngine(int levelNumber, int checkpoint);
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


private:
	float getMaximalHoleRadius() const; // used for DeathClose and DeathOpen

	shared_ptr<WapWwd> _wwd;
	vector<shared_ptr<LevelPlane>> _planes;
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
