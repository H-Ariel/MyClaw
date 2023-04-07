#include "Gabriel.h"
#include "../Player.h"
#include "../AssetsManager.h"


static bool riseCannon = false;
static bool operateSwitch = false;
static bool operateCannon = false;
static bool resetCannon = false;


#define GABRIAL_ANI_CHANGE_CANNON_SWITCH _animations["STRIKE6"] // TODO: rename
#define ANIMATION_IDLE _animations["IDLE1"]


// TODO: continue write Gabriel class


Gabriel::Gabriel(const WwdObject& obj, Player* player)
	: BaseBoss(obj, player, 10, "", "HITHIGH", "HITLOW", "KILLFALL4", "", "", ""),
	_switchCannonTime(0)
{
	_forward = false;
	_ani = ANIMATION_IDLE;
}

void Gabriel::Logic(uint32_t elapsedTime)
{
	PreLogic(elapsedTime);

	if (_ani != GABRIAL_ANI_CHANGE_CANNON_SWITCH)
	{
		_switchCannonTime += elapsedTime;
	}
	
	if (_switchCannonTime >= 3000) // use cannon every 3 seconds
	{
		_switchCannonTime = 0;
		operateSwitch = true;
		_ani = GABRIAL_ANI_CHANGE_CANNON_SWITCH;
		_forward = true;
		_ani->reset();
	}

	if (_ani == GABRIAL_ANI_CHANGE_CANNON_SWITCH && _ani->isFinishAnimation())
	{
		_ani = ANIMATION_IDLE;
		_forward = false;
	//	resetCannon = true;
		_ani->reset();
	}


	PostLogic(elapsedTime);
}

pair<Rectangle2D, int8_t> Gabriel::GetAttackRect()
{
	return {};
}

void Gabriel::makeAttack()
{
}

bool Gabriel::checkForHurts()
{
	// TODO: block all CC attacks
	return BaseEnemy::checkForHurts();
}




GabrielCannon::GabrielCannon(const WwdObject& obj, Player* player)
	: BaseStaticPlaneObject(obj, player)
{
	string aniSet = PathManager::getAnimationSetPath(obj.imageSet);
	string imageSet = PathManager::getImageSetPath(obj.imageSet);

	_home = AssetsManager::loadAnimation(aniSet + "/HOME.ANI", imageSet); // nop
	_rest = AssetsManager::loadAnimation(aniSet + "/REST.ANI", imageSet); // go down
	_rise = AssetsManager::loadAnimation(aniSet + "/RISE.ANI", imageSet); // go up
	_horzfire = AssetsManager::loadAnimation(aniSet + "/HORZFIRE.ANI", imageSet); // horizontal fire
	_vertfire = AssetsManager::loadAnimation(aniSet + "/VERTIFIRE.ANI", imageSet); // vertical fire

	_ani = _home;

	_ani->position = position;
}

void GabrielCannon::Logic(uint32_t elapsedTime)
{
	const shared_ptr<const Animation> prevAni = _ani;

	if (_ani == _rest && _ani->isFinishAnimation())
	{
		_ani = _home;
	}
	else if ((_ani == _horzfire || _ani == _vertfire) && _ani->isFinishAnimation())
	{
		resetCannon = true;
	}


	if (riseCannon)
	{
		_ani = _rise;
		riseCannon = false;
	}

	if (operateCannon)
	{
		if (_ani == _rise)
		{
			_ani = _horzfire;
		}
		else
		{
			_ani = _vertfire;
		}
		operateCannon = false;
	}

	if (resetCannon)
	{
		if (_ani == _rise)
		{
			_ani = _rest;
		}
		else
		{
 			_ani = _home;
		}

		resetCannon = false;
	}
	
	if (prevAni != _ani)
	{
		_ani->reset();
		_ani->loopAni = false;
		_ani->position = position;
	}

	_ani->Logic(elapsedTime);
}





CannonSwitch::CannonSwitch(const WwdObject& obj, Player* player)
	: BaseStaticPlaneObject(obj, player)
{
	_ani = AssetsManager::loadAnimation(PathManager::getAnimationSetPath(obj.imageSet) + "/SWITCH.ANI", PathManager::getImageSetPath(obj.imageSet));
	_ani->position = position;
	_ani->updateFrames = false;
}

void CannonSwitch::Logic(uint32_t elapsedTime)
{
	if (operateSwitch)
	{
		operateSwitch = false;
		operateCannon = true;
		_ani->reset();
		_ani->position = position;
		_ani->loopAni = false;
	}

	_ani->Logic(elapsedTime);
}





CannonButton::CannonButton(const WwdObject& obj, Player* player)
	: BaseStaticPlaneObject(obj, player), _pressedTime(0)
{
	_idle = AssetsManager::loadAnimation(PathManager::getAnimationSetPath(obj.imageSet) + "/IDLE.ANI", PathManager::getImageSetPath(obj.imageSet));
	_pressed = AssetsManager::createAnimationFromFromPidImage(PathManager::getImageSetPath(obj.imageSet) + "/001.PID");

	_ani = _idle;

	setObjectRectangle();
}

void CannonButton::Logic(uint32_t elapsedTime)
{
	_ani->Logic(elapsedTime);

	if (_ani == _pressed )
	{
		_pressedTime += elapsedTime;

		if (_pressedTime >= 2500)
		{
			_ani = _idle;
			_pressedTime = 0;
		}
	}
	else
	{
		if (
			_player->GetRect().intersects(_objRc) ||
			_player->GetAttackRect().first.intersects(_objRc)
			)
		{
			_ani = _pressed;
			riseCannon = true;
		}
	}
}
