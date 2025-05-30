#pragma once

#include "BasePlaneObject.h"


// simple animation that plays only once
class OneTimeAnimation : public BasePlaneObject
{
public:
	OneTimeAnimation(D2D1_POINT_2F pos, shared_ptr<UIAnimation> ani, bool removeAtEnd = true);
	OneTimeAnimation(D2D1_POINT_2F pos, const string& aniPath, const string& imageSet = "", bool removeAtEnd = true);
	void Logic(uint32_t elapsedTime) override;
	void Draw() override;

private:
	bool _removeAtEnd;
};

