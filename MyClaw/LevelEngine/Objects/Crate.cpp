#include "Crate.h"
#include "Projectiles/EnemyProjectile.h"
#include "../GlobalObjects.h"
#include "../CheatsManager.h"
#include "PowderKeg.h"

constexpr int CRATE_HEIGHT = 43;


Crate::Crate(const WwdObject& obj)
	: BaseStaticPlaneObject(obj, false)
{
	if (obj.powerup > 0) _itemsTypes.push_back(obj.powerup);
	if (obj.userRect1.left > 0) _itemsTypes.push_back(obj.userRect1.left);
	if (obj.userRect1.right > 0) _itemsTypes.push_back(obj.userRect1.right);
	if (obj.userRect1.bottom > 0) _itemsTypes.push_back(obj.userRect1.bottom);
	if (obj.userRect1.top > 0) _itemsTypes.push_back(obj.userRect1.top);
	if (obj.userRect2.left > 0) _itemsTypes.push_back(obj.userRect2.left);
	if (obj.userRect2.right > 0) _itemsTypes.push_back(obj.userRect2.right);
	if (obj.userRect2.bottom > 0) _itemsTypes.push_back(obj.userRect2.bottom);
	if (obj.userRect2.top > 0) _itemsTypes.push_back(obj.userRect2.top);
	if (_itemsTypes.size() == 0) _itemsTypes.push_back(Item::Type::Treasure_Coins);

	_ani = AssetsManager::getCopyAnimationFromDirectory(PathManager::getImageSetPath(obj.imageSet));
	_ani->updateFrames = false;
	_ani->loopAni = false;

	setObjectRectangle();
}
void Crate::Logic(uint32_t elapsedTime)
{
	_ani->Logic(elapsedTime);


	if (GO::isPlayerAttack() && _objRc.intersects(GO::getPlayerAttackRect().first))
	{
		_ani->updateFrames = true;
	}
	else
	{
	 	const vector<Projectile*>& projectiles = GO::getActionPlaneProjectiles();
	 	auto proj = find_if(projectiles.begin(), projectiles.end(), [&](Projectile* p) {
	 		return (!p->isClawDynamite() || p->getDamage() != 0) && _objRc.intersects(p->GetRect());
	 	});
	 	if (proj != projectiles.end())
	 	{
	 		_ani->updateFrames = true;
	 		if ((*proj)->isClawBullet() || isinstance<EnemyProjectile>(*proj))
	 			(*proj)->removeObject = true; // bullets don't keep flying, but magic does
	 	}

		if (!_ani->updateFrames)
		{
			const vector<PowderKeg*>& powderKegs = GO::getActionPlanePowderKegs();
			_ani->updateFrames = any_of(powderKegs.begin(), powderKegs.end(), [&](PowderKeg* p) {
				return p->getDamage() > 0 && _objRc.intersects(p->GetRect());
			});
		}
	}

	removeObject = _ani->isFinishAnimation();
}
vector<BasePlaneObject*> Crate::getItems()
{
	vector<BasePlaneObject*> items;

	if (!_itemsTypes.empty())
	{
		WwdObject newObj;
		newObj.x = (int32_t)position.x;
		newObj.y = (int32_t)position.y;
		newObj.z = logicZ;

		for (int8_t t : _itemsTypes)
		{
			Item* i = Item::getItem(newObj, t);
			i->speed.y = -0.6f;
			if (GO::cheats->isMultiTreasures())
				i->speed.x = getRandomFloat(-0.25f, 0.25f); // treasures are scattered around
			items.push_back(i);
		}

		if (!GO::cheats->isMultiTreasures())
			_itemsTypes.clear(); // do not collect them again
	}

	return items;
}

StackedCrates::StackedCrates(const WwdObject& obj)
	: BaseStaticPlaneObject(obj, false)
{
	int height = obj.height;
	if (height < 1) height = 1;
	else if (height > 8) height = 8;

	for (int crateIdx = 0; crateIdx < height; crateIdx++)
	{
		WwdObject newObj = obj;
		newObj.y -= crateIdx * CRATE_HEIGHT;
		newObj.z += crateIdx;
		newObj.userRect1 = newObj.userRect2 = {};

		/*
		if (crateIdx == 0) newObj.userRect1.left = obj.userRect1.left;
		else if (crateIdx == 1) newObj.userRect1.top = obj.userRect1.top;
		else if (crateIdx == 2) newObj.userRect1.right = obj.userRect1.right;
		else if (crateIdx == 3) newObj.userRect1.bottom = obj.userRect1.bottom;
		else if (crateIdx == 4) newObj.userRect2.left = obj.userRect2.left;
		else if (crateIdx == 5) newObj.userRect2.top = obj.userRect2.top;
		else if (crateIdx == 6) newObj.userRect2.right = obj.userRect2.right;
		else if (crateIdx == 7) newObj.userRect2.bottom = obj.userRect2.bottom;
		*/
		*(&newObj.userRect1.left + crateIdx) = *(&obj.userRect1.left + crateIdx); // match treasure by index

		crates.push_back(make_shared<Crate>(newObj));
	}
}
void StackedCrates::Logic(uint32_t elapsedTime)
{
	removeObject = crates.empty();

	if (!removeObject)
	{
		// drop the crates (save the stack without holes)
		for (size_t i = 0; i < crates.size(); i++)
		{
			crates[i]->position.y = position.y - i * CRATE_HEIGHT;
			crates[i]->setObjectRectangle();
		}

		for (shared_ptr<Crate>& c : crates)
		{
			c->Logic(elapsedTime);
		}
	}
}
void StackedCrates::Draw()
{
	for (const shared_ptr<Crate>& c : crates)
	{
		c->Draw();
	}
}
vector<BasePlaneObject*> StackedCrates::getItems()
{
	vector<BasePlaneObject*> allItems;
	for (size_t i = 0; i < crates.size(); i++)
	{
		if (crates[i]->isBreaking())
		{
			vector<BasePlaneObject*> items = crates[i]->getItems();

			for (BasePlaneObject* i : items)
				((Item*)i)->speed.x = getRandomFloat(-0.25f, 0.25f);

			allItems += items;
		}
		if (crates[i]->removeObject)
		{
			crates.erase(crates.begin() + i);
			i--; // cancel `i++`
		}
	}
	return allItems;
}
