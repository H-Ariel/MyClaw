#pragma once

#include "LevelEndEngine.h"


class LevelEndEngineState
{
public:
	LevelEndEngineState(LevelEndEngine* levelEndEngine)
		: _levelEndEngine(levelEndEngine), logicDone(false) {}

	virtual void Logic() = 0;
	virtual LevelEndEngineState* nextState() = 0;

	bool logicDone;

protected:
	LevelEndEngine* _levelEndEngine;
};


// small macro for big code :)
#define DefineLevelEndEngineState(stateName) \
class stateName : public LevelEndEngineState { \
public: \
	stateName(LevelEndEngine* levelEndEngine) : LevelEndEngineState(levelEndEngine) {} \
	void Logic() override; \
	LevelEndEngineState* nextState() override; \
}

DefineLevelEndEngineState(StartState);
DefineLevelEndEngineState(DrawScoreState);
DefineLevelEndEngineState(WaitState);
DefineLevelEndEngineState(EndState);
