#pragma once

#include "GameEngine/BaseEngine.h"
#include "LevelHUD.h"


class LevelPlane;
class ActionPlane;
class ClawLevelEngine;

struct ClawLevelEngineFields
{
	ClawLevelEngineFields(int levelNumber, ClawLevelEngine* clawLevelEngine);
	~ClawLevelEngineFields();

	shared_ptr<WapWwd> _wwd;
	vector<shared_ptr<LevelPlane>> _planes;
	ActionPlane* actionPlane;
	LevelHUD* _hud;
	ColorF _saveBgColor;
	float _saveWindowScale;
};

class ClawLevelEngineState {
public:
	enum class StateType : int8_t {
		Play,
		DeathFall,
		DeathClose,
		DeathOpen,
		WrapClose,
		WrapOpen,
		GameOver
	};

	virtual ~ClawLevelEngineState() = default;
	virtual void Logic(uint32_t elapsedTime) = 0;

	StateType getType() const { return _type; }

protected:
	ClawLevelEngineState(StateType type, ClawLevelEngine* clawLevelEngine)
		: _type(type), _clawLevelEngine(clawLevelEngine) {}

	const StateType _type;
	ClawLevelEngine* _clawLevelEngine;
};

class ClawLevelEngine : public BaseEngine
{
public:
	ClawLevelEngine(int levelNumber, int checkpoint);
	ClawLevelEngine(shared_ptr<ClawLevelEngineFields> fields);
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
	void init(); // call this in each constructor

	float getInitialHoleRadius() const;

	// TODO: try do not save fields seperate. need find better solution
	shared_ptr<ClawLevelEngineFields> _fields; // save fields for easy access after ingame-menu
	
	float _holeRadius; // the radius of the hole that remains until closed
	
	D2D1_POINT_2F _wrapDestination;
	float _wrapCoverTop ; // used in wrap transition animation
	float _bossWarpX;
	bool _isBossWarp;

	int _gameOverTimeCounter; // used to delay game over screen

	ClawLevelEngineState* _state;


	friend class PlayState;
	friend class DeathFallState;
	friend class DeathCloseState;
	friend class DeathOpenState;
	friend class WrapCloseState;
	friend class WrapOpenState;
	friend class GameOverState;
};
