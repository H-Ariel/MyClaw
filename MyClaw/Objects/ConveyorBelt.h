#pragma once

#include "BasePlaneObject.h"
#include "GameEngine/WindowManager.h"

class ConveyorBelt : public BaseStaticPlaneObject
{
public:
	ConveyorBelt(const WwdObject& obj);
	void Logic(uint32_t elapsedTime) override;
	void Draw() override {
		BaseStaticPlaneObject::Draw();
		WindowManager::drawRect(GetRect(), ColorF::Red,3);
	}

	void orderAnimation(int n);

private:
	float speed;
	const bool _canMoveCC;
};
