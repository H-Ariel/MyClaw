#include "Warp.h"
#include "../Player.h"

// TODO: insert it into `Item`
Warp::Warp(const WwdObject& obj, Player* player, int8_t type)
	: Item(obj, player, type), destination({ (float)obj.speedX, (float)obj.speedY }), oneTimeWarp(obj.smarts == 0)
{
}
void Warp::Logic(uint32_t elapsedTime)
{
	if (CollisionDistances::isCollision(GetRect(), _player->GetRect()))
	{
		_player->position = destination;
		_player->stopFalling(0);
		removeObject = oneTimeWarp;
		// TODO: cool animation
	}
}
