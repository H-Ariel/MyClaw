#pragma once

#include "../BasePlaneObject.h"


class Checkpoint : public BaseStaticPlaneObject
{
public:
	Checkpoint(const WwdObject& obj, Player* player);
	void Logic(uint32_t elapsedTime) override;

protected:
	Checkpoint(const WwdObject& obj, Player* player, string imageSetPath);

private:
	const string _imageSetPath;
	enum class States { Down, Rise, Wave };
	States _state;
};

class SuperCheckpoint : public Checkpoint
{
public:
	SuperCheckpoint(const WwdObject& obj, Player* player);

	void Logic(uint32_t elapsedTime) override;
};
