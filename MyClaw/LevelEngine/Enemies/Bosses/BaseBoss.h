#pragma once

#include "../BaseEnemy.h"


class BaseBoss : public BaseEnemy
{
public:
	BaseBoss(const WwdObject& obj, const string& fallDeadAni = "");
	~BaseBoss();

	virtual void Logic(uint32_t elapsedTime) override = 0;

protected:
	virtual bool checkForHurts() override = 0; // We added this function because bosses are not hit by CC projectiles

	int _hitsCuonter; // count the times CC hit the boss
	bool _blockClaw, _canJump;

private:
	const D2D1_POINT_2L _gemPos;
};
