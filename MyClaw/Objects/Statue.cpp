#include "Statue.h"


Statue::Statue(const WwdObject& obj, Player* player)
	: Crate(obj, player)
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
vector<Item*> Statue::getItems()
{
	vector<Item*> items = Crate::getItems();

	for (Item* i : items)
	{
		i->setSpeedX(getRandomFloat(-0.25f, 0.25f) * (getRandomInt(0, 1) == 1 ? 1 : -1));
	}

	return items;
}