#pragma once

#include "../BasePlaneObject.h"


class Checkpoint : public BaseStaticPlaneObject
{
public:
	static Checkpoint* createCheckpoint(const WwdObject& obj, Player* player, bool isSuperCheckpoint);
	
	void Logic(uint32_t elapsedTime) override;

private:
	Checkpoint(const WwdObject& obj, Player* player, string imageSetPath, bool isSuperCheckpoint);

	enum class States : int8_t { Down, Rise, Wave };
	const string _imageSetPath;
	States _state;
	const bool _isSuperCheckpoint;
};
