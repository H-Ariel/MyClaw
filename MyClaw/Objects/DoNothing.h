#pragma once

#include "BasePlaneObject.h"


class DoNothing : public BaseStaticPlaneObject // it's also `...Candy`
{
public:
	DoNothing(const WwdObject& obj);
};
