#include "Statue.h"


Statue::Statue(const WwdObject& obj)
	: Crate(obj)
{
}
void Statue::Logic(uint32_t elapsedTime)
{
	Crate::Logic(elapsedTime);
	if (removeObject)
	{
		removeObject = false;
		_ani->updateFrames = false;
	}
}
vector<BasePlaneObject*> Statue::getItems()
{
	vector<BasePlaneObject*> items = Crate::getItems();

	for (BasePlaneObject* i : items)
	{
		((Item*)i)->speed.x = getRandomFloat(-0.25f, 0.25f);
	}

	return items;
}