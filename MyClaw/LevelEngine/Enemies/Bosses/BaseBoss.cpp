#include "BaseBoss.h"
#include "../EnemyTools.h"
#include "../../GlobalObjects.h"


BaseBoss::BaseBoss(const WwdObject& obj, const string& fallDeadAni)
	: BaseEnemy(obj, obj.health, 10, "", "HITHIGH", "HITLOW", fallDeadAni,
		"", "", "", "", "", ENEMY_PATROL_SPEED, true),
	_hitsCuonter(1), _blockClaw(false), _canJump(true), _gemPos({ obj.speedX, obj.speedY })
{
	if (fallDeadAni.empty())
		_fallDead = false;

	_ani = _aniIdle;
}
BaseBoss::~BaseBoss()
{
	if (removeObject)
	{
		WwdObject obj;
		obj.x = (int32_t)position.x;
		obj.y = (int32_t)position.y;
		obj.minX = _gemPos.x;
		obj.minY = _gemPos.y;
		obj.imageSet = "LEVEL_GEM";
		GO::addObjectToActionPlane(DBG_NEW BossGem(obj));
	}
}
