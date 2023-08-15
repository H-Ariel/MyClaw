#include "RedTail.h"
#include "../Assets-Managers/AssetsManager.h"
#include "../Player.h"


RedTailSpikes::RedTailSpikes(const WwdObject& obj)
	: BaseDamageObject(obj, 20)
{
	_ani = AssetsManager::createAnimationFromDirectory(PathManager::getImageSetPath(obj.imageSet), 0, false);
	setObjectRectangle();
}
bool RedTailSpikes::isDamage() const { return true; }


// TODO: find when we activate the wind
RedTailWind::RedTailWind(const WwdObject& obj)
	: BasePlaneObject(obj), _windTimeCounter(0) {}
void RedTailWind::Logic(uint32_t elapsedTime)
{
	if (_windTimeCounter > 0)
	{
		_windTimeCounter -= elapsedTime;
		player->position.x -= 0.1f * elapsedTime;
	}
}
void RedTailWind::activate() { _windTimeCounter = 1000; }
