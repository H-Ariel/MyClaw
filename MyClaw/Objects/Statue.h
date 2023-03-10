#pragma once

#include "Crate.h"


class Statue : public Crate
{
public:
	Statue(const WwdObject& obj, Player* player);
	void Logic(uint32_t elapsedTime) override;
	vector<Item*> getItems() override;
};
