#pragma once

#include "ClawLevelEngine.h"


class ClawLevelEngineState {
public:
	virtual ~ClawLevelEngineState() = default;
	virtual void Logic(uint32_t elapsedTime) = 0;
	virtual void Draw();


protected:
	ClawLevelEngineState(ClawLevelEngine* clawLevelEngine)
		: _levelEngine(clawLevelEngine) {}

	ClawLevelEngine* _levelEngine;
};


class PlayState : public ClawLevelEngineState
{
public:
	PlayState(ClawLevelEngine* clawLevelEngine);
	void Logic(uint32_t elapsedTime) override;
};

class DeathFallState : public ClawLevelEngineState
{
public:
	DeathFallState(ClawLevelEngine* clawLevelEngine)
		: ClawLevelEngineState(clawLevelEngine) {}
	void Logic(uint32_t elapsedTime) override;
};

class DeathCloseState : public ClawLevelEngineState
{
public:
	DeathCloseState(ClawLevelEngine* clawLevelEngine);
	void Logic(uint32_t elapsedTime) override;
	void Draw() override;

private:
	float _holeRadius; // the radius of the hole that remains until closed
};

class DeathOpenState : public ClawLevelEngineState
{
public:
	DeathOpenState(ClawLevelEngine* clawLevelEngine)
		: ClawLevelEngineState(clawLevelEngine), _holeRadius(0) {}
	void Logic(uint32_t elapsedTime) override;
	void Draw() override;

private:
	float _holeRadius; // the radius of the hole that remains until opened
};

class WrapCloseState : public ClawLevelEngineState
{
public:
	WrapCloseState(ClawLevelEngine* clawLevelEngine, Warp* destinationWarp);
	void Logic(uint32_t elapsedTime) override;
	void Draw() override;

private:
	D2D1_POINT_2F _wrapDestination; // save destination to set it after animation
	float _wrapCoverTop; // used in wrap transition animation
	float _bossWarpX;
	bool _isBossWarp;
};

class WrapOpenState : public ClawLevelEngineState
{
public:
	WrapOpenState(ClawLevelEngine* clawLevelEngine)
		: ClawLevelEngineState(clawLevelEngine), _wrapCoverTop(0) {}
	void Logic(uint32_t elapsedTime) override;
	void Draw() override;

private:
	float _wrapCoverTop; // used in wrap transition animation
};

class GameOverState : public ClawLevelEngineState
{
public:
	GameOverState(ClawLevelEngine* clawLevelEngine);
	void Logic(uint32_t elapsedTime) override;
	void Draw() override;

private:
	int _gameOverTimeCounter; // used to delay game over screen
};
