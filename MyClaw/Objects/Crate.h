#pragma once

#include "Item.h"


class Crate : public BaseStaticPlaneObject
{
public:
	Crate(const WwdObject& obj, Player* player);
	void Logic(uint32_t elapsedTime) override;
	bool isCracking() const;
	vector<Item*> getItems();

private:
	vector<int8_t> _itemsTypes; // contents of the crate
	bool _flag; // store if the items were taken from this crate. TODO: rename

	friend class StackedCrates;
};

class StackedCrates : public BaseStaticPlaneObject
{
public:
	StackedCrates(const WwdObject& obj, Player* player);
	void Logic(uint32_t elapsedTime) override;
	void Draw() override;

	vector<Item*> getItems();

private:
	vector<shared_ptr<Crate>> crates;
};
