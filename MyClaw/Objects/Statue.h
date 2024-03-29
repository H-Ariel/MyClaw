#pragma once

#include "Crate.h"


class Statue : public Crate
{
public:
	Statue(const WwdObject& obj);
	void Logic(uint32_t elapsedTime) override;
	vector<BasePlaneObject*> getItems() override;
};
