#pragma once

#include "Projectile.h"


class EnemyProjectile : public Projectile
{
public:
	EnemyProjectile(const WwdObject& obj, const string& projectileAniDir);
};

class RatBomb : public Projectile
{
public:
	RatBomb(const WwdObject& obj);
	~RatBomb();
};

class CrabBomb : public Projectile // TODO: combine with `RatBomb`
{
public:
	CrabBomb(const WwdObject& obj);
	~CrabBomb();
};

class GabrielBomb : public Projectile // TODO: combine with `RatBomb`
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
