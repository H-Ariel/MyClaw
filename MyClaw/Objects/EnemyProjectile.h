#pragma once

#include "Projectile.h"


class EnemyProjectile : public Projectile
{
public:
	EnemyProjectile(const WwdObject& obj, const string& projectileAniDir);
	EnemyProjectile(shared_ptr<Animation> ani, int damage, D2D1_POINT_2F speed, D2D1_POINT_2F initialPosition);
};

class RatBomb : public Projectile
{
public:
	RatBomb(const WwdObject& obj);
	~RatBomb();
};

class CrabBomb : public Projectile
{
public:
	CrabBomb(const WwdObject& obj);
	~CrabBomb();
};

class GabrielBomb : public Projectile
{
public:
	GabrielBomb(const WwdObject& obj);
	~GabrielBomb();
	void Logic(uint32_t elapsedTime) override;
	void stopFalling(float collisionSize) override;
};

class CannonBall : public Projectile
{
public:
	CannonBall(const WwdObject& obj);
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
	SirenProjectile(const WwdObject& obj, int32_t delay);
	void Logic(uint32_t elapsedTime) override;
	void Draw() override;

private:
	int32_t _delay; // in milliseconds
};


// create new class for TProjectilesShooter
class TProjectile : public Projectile
{
public:
	TProjectile(shared_ptr<Animation> ani, int damage, D2D1_POINT_2F speed, D2D1_POINT_2F initialPosition);
};
