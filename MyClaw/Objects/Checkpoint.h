#pragma once

#include "BasePlaneObject.h"


class Checkpoint : public BaseStaticPlaneObject
{
public:
	Checkpoint(const WwdObject& obj, int levelNumber);
	void Logic(uint32_t elapsedTime) override;

private:
	enum class States : int8_t { Down, Rise, Wave };
	const string _imageSetPath;
	int _levelNumber, _superCheckpoint;
	States _state;
	bool _isSaved;
};
