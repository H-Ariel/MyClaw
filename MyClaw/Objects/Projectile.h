#pragma once

#include "../BasePlaneObject.h"

#define DEFAULT_PROJECTILE_SPEED 400


class ClawDynamite;


class Projectile : public BaseDynamicPlaneObject
{
public:
	Projectile(const WwdObject& obj, int8_t damage, string aniDirPath);
	void Logic(uint32_t elapsedTime) override;

	void bounceTop() override;
	void stopFalling(float collisionSize) override;

	bool isClawBullet() const; // pistol shot
	bool isClawDynamite() const { return isinstance<ClawDynamite>(this); }

	virtual int8_t getDamage() const;

private:
	const int8_t _damage;
};


class ClawProjectile : public Projectile
{
public:
	enum class Types : int8_t
	{
		Pistol, // pistol shot
		Magic, // magic claw
		Dynamite,
		FireSword,
		IceSword,
		LightningSword,
		None
	};
	static ClawProjectile* createNew(Types type, const WwdObject& data);

	const Types type;

protected:
	ClawProjectile(const WwdObject& obj, int8_t damage, string aniDirPath, Types type);
};

class ClawDynamite : public ClawProjectile
{
public:
	ClawDynamite(const WwdObject& obj);
	void Logic(uint32_t elapsedTime) override;
	void stopFalling(float collisionSize) override;
	void stopMovingLeft(float collisionSize) override;
	void stopMovingRight(float collisionSize) override;
	void bounceTop() override;
	int8_t getDamage() const override;

private:
	enum class State : int8_t { Fly, Wait, Explos };
	int32_t _delayBeforeExplos; // in milliseconds
	State _state;
};


class EnemyProjectile : public Projectile
{
public:
	EnemyProjectile(const WwdObject& obj, string projectileAniDir);
};

class RatBomb : public Projectile
{
public:
	RatBomb(const WwdObject& obj);
	~RatBomb();
};

class CannonBall : public Projectile
{
public:
	CannonBall(const WwdObject& obj);
};


bool isProjectile(BasePlaneObject* obj);
bool isClawProjectile(BasePlaneObject* obj);
bool isEnemyProjectile(BasePlaneObject* obj);
