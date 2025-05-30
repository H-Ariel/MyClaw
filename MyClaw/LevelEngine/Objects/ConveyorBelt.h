#pragma once

#include "BasePlaneObject.h"


class ConveyorBelt : public BaseStaticPlaneObject
{
public:
	ConveyorBelt(const WwdObject& obj);
	void Logic(uint32_t elapsedTime) override;

	void Draw() override;

	bool hasSameMovement(ConveyorBelt* belt) const;
	void extend(ConveyorBelt* belt);

	float getSpeed() const { return _canMoveCC ? _speed : 0; }

private:
	vector<shared_ptr<UIAnimation>> _animations; // list of animation for whole belt (sequence)
	const float _speed;
	const bool _canMoveCC;
};
