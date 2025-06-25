#include "Aquatis.h"
#include "AquatisTentacle.h"
#include "AquatisStalactite.h"
#include "../../../GlobalObjects.h"
#include "../../../Objects/OneTimeAnimation.h"


#define ANIMATION_HITHIGH	_animations["HITHIGH"]
#define ANIMATION_HITLOW	_animations["HITLOW"]
#define ANIMATION_KILLFALL	_animations["KILLFALL"]
#define ANIMATION_PUNCH		_animations["PUNCH"]
#define ANIMATION_IDLE		_animations["IDLE1"]


Aquatis* Aquatis::_Aquatis = nullptr;

Aquatis* Aquatis::createAquatis(const WwdObject& obj) {
	/*if (_Aquatis != nullptr) {
		LOG("[WARNING] more than one Aquatis");
		return _Aquatis;
	}*/
	_Aquatis = DBG_NEW Aquatis(obj);
	return _Aquatis;
}

Aquatis::Aquatis(const WwdObject& obj)
	: BaseBoss(obj), AquatisStalactitesList({}), currStalactiteIdx(0)
{
	_ani = ANIMATION_IDLE;
	_isMirrored = true;
	_health = STALACTITES_COUNT;

	// create punch animation (punch up and punch down)
	_animations["PUNCH"] = make_shared<UIAnimation>(_animations["STRIKE1"]->getFramesList() + _animations["STRIKE2"]->getFramesList());
	_animations.erase("STRIKE1"); _animations.erase("STRIKE2");
}
Aquatis::~Aquatis()
{
	if (removeObject)
	{
		GO::addObjectToActionPlane(DBG_NEW OneTimeAnimation(position, ANIMATION_KILLFALL));
		for (AquatisTentacle* i : AquatisTentaclesList)
			i->removeObject = true;
	}

	_Aquatis = nullptr;
}
void Aquatis::Logic(uint32_t elapsedTime)
{
	if (currStalactiteIdx == STALACTITES_COUNT)
	{
		removeObject = true;
		return;
	}

	if (_ani == ANIMATION_HITHIGH || _ani == ANIMATION_HITLOW) // hit animation
	{
		if (!_ani->isFinishAnimation())
		{
			PostLogic(elapsedTime);
			return;
		}
		else
		{
			_ani = ANIMATION_PUNCH;
			_ani->reset();

			_isAttack = true;
		}
	}

	if (_ani == ANIMATION_PUNCH && !_ani->isFinishAnimation())
	{
		PostLogic(elapsedTime);
		_isAttack = !_ani->isFinishAnimation();
		return;
	}
	else
	{
		_ani = ANIMATION_IDLE;
	}

	// check is stalactite touch Aquatis
	AquatisStalactite* stalactite = AquatisStalactitesList[currStalactiteIdx];
	if (this->GetRect().intersects(stalactite->GetRect()))
	{
		stalactite->stopFalling(0);
		_ani = getRandomInt(0, 1) == 1 ? ANIMATION_HITLOW : ANIMATION_HITHIGH;
		_ani->reset();
		_ani->loopAni = false;
		_health -= 1;
	}

	PostLogic(elapsedTime);
}
pair<Rectangle2D, int> Aquatis::GetAttackRect()
{
	if (_ani == ANIMATION_PUNCH)
	{
		Rectangle2D rc = _ani->GetRect();
		
		if (_ani->getFrameNumber() > 4) // punch down
			rc.top += 80;

		return { rc, 10 };
	}
	return pair<Rectangle2D, int>();
}
bool Aquatis::checkForHurts()
{
	return false;
}


void Aquatis::activateNextStalactite() {
	AquatisStalactitesList[currStalactiteIdx]->activate();
	currStalactiteIdx += 1;
}

AquatisTentacle* Aquatis::addTentacle(const WwdObject& obj)
{
	AquatisTentacle* tentacle = DBG_NEW AquatisTentacle(obj);
	AquatisTentaclesList.push_back(tentacle);
	return tentacle;
}
AquatisStalactite* Aquatis::addStalactite(const WwdObject& obj)
{
	AquatisStalactite* stalactite = DBG_NEW AquatisStalactite(obj);
	AquatisStalactitesList[obj.smarts - 1] = stalactite;
	return stalactite;
}
