#include "EnemyHelp.h"
#include "WindowManager.h"
#include "Player.h"


#define GEM_SPEED 0.2f


DeadEnemy::DeadEnemy(const WwdObject& obj, shared_ptr<Animation> deadAni)
	: BaseDynamicPlaneObject(obj)
{
	_ani = deadAni;
	speed.x = obj.speedX / 1000.f;
	speed.y = obj.speedY / 1000.f;
	_isMirrored = obj.speedX < 0;
}
void DeadEnemy::Logic(uint32_t elapsedTime)
{
	position.x += speed.x * elapsedTime;
	speed.y += GRAVITY * elapsedTime;
	position.y += speed.y * elapsedTime;
	_ani->position = position;
	_ani->Logic(elapsedTime);
	removeObject = !WindowManager::isInScreen(_ani->GetRect());
}

BossGem::BossGem(const WwdObject& obj)
	: Item(obj, Item::Type::NineLivesGem), _destination({ (float)obj.minX, (float)obj.minY })
{
	float a = atan((position.y - _destination.y) / (position.x - _destination.x));

	speed.x = cos(a) * GEM_SPEED;
	speed.y = sin(a) * GEM_SPEED;

	if (_destination.x == position.x)
	{
		speed.x = 0;
		speed.y = GEM_SPEED;
	}
	else if (_destination.x < position.x)
	{
		speed.x = -speed.x;
		speed.y = -speed.y;
	}
}
void BossGem::Logic(uint32_t elapsedTime)
{
	if (speed.x == 0 && speed.y == 0)
	{
		if (GetRect().intersects(player->GetRect()))
		{
			player->collectItem(this);
		}
	}
	else
	{
		if (abs(position.x - _destination.x) <= 5 && abs(position.y - _destination.y) <= 5)
		{
			position = _destination;
			speed = {};
		}
		else
		{
			position.y += speed.y * elapsedTime;
			position.x += speed.x * elapsedTime;
		}
	}
}
