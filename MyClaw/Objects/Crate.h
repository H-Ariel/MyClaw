#pragma once

#include "Item.h"


// TODO: maybe it should be BaseDynamicPlaneObject, because it can fall down after lower crate is broken
class Crate : public BaseStaticPlaneObject
{
public:
	Crate(const WwdObject& obj);
	void Logic(uint32_t elapsedTime) override;
	bool isBreaking() const { return _ani->updateFrames; }
	virtual vector<BasePlaneObject*> getItems();

private:
	vector<int8_t> _itemsTypes; // contents of the crate

	friend class StackedCrates;
};

class StackedCrates : public BaseStaticPlaneObject
{
public:
	StackedCrates(const WwdObject& obj);
	void Logic(uint32_t elapsedTime) override;
	void Draw() override;

	vector<BasePlaneObject*> getItems();

private:
	vector<shared_ptr<Crate>> crates;
};
