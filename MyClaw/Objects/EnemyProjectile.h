#pragma once

#include "Projectile.h"


class EnemyProjectile : public Projectile
{
public:
	EnemyProjectile(const WwdObject& obj, const string& projectileAniDir);
	EnemyProjectile(const WwdObject& obj, const string& projectileAni, const string& imageSet);
	EnemyProjectile(shared_ptr<UIAnimation> ani, int damage, D2D1_POINT_2F speed, D2D1_POINT_2F initialPosition);
};


// not all of these projectiles are enemy-projectiles, but all of them can hurt CC
// TODO split this file...


class RatBomb : public EnemyProjectile
{
public:
	RatBomb(const WwdObject& obj);
	~RatBomb();
};

class CannonBall : public EnemyProjectile
{
public:
	CannonBall(const WwdObject& obj);
};

class CrabBomb : public EnemyProjectile
{
public:
	CrabBomb(const WwdObject& obj);
	~CrabBomb();
};

class TProjectile : public EnemyProjectile
{
public:
	TProjectile(shared_ptr<UIAnimation> ani, int damage, D2D1_POINT_2F speed, D2D1_POINT_2F initialPosition);
};

class MercatTrident : public EnemyProjectile
{
public:
	MercatTrident(const WwdObject& obj);
	~MercatTrident();
};

class SirenProjectile : public EnemyProjectile
{
public:
	SirenProjectile(const WwdObject& obj, int delay);
	void Logic(uint32_t elapsedTime) override;
	void Draw() override;

private:
//	int _delay; // in milliseconds
};

class LavahandProjectile : public EnemyProjectile
{
public:
	LavahandProjectile(D2D1_POINT_2F initialPosition, bool mirrored);
	~LavahandProjectile();
	void Logic(uint32_t elapsedTime) override;
	void stopFalling(float collisionSize) override;
};
