#pragma once

#include "../BasePlaneObject.h"

#define DEFAULT_PROJECTILE_SPEED 400


class ClawDynamite;


class Projectile : public BaseDynamicPlaneObject
{
public:
	Projectile(const WwdObject& obj, const string& aniDirPath, const string& imageSet = "");
	void Logic(uint32_t elapsedTime) override;

	void bounceTop() override;
	void stopFalling(float collisionSize) override;

	bool isClawBullet() const; // pistol shot
	bool isClawDynamite() const { return isinstance<ClawDynamite>(this); }

	virtual int getDamage() const;

protected:
	Projectile(shared_ptr<Animation> ani, int damage, D2D1_POINT_2F speed, D2D1_POINT_2F initialPosition);

private:
	int32_t _timeLeft; // time left before projectile disappears
	const int _damage;
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
	ClawProjectile(const WwdObject& obj, const string& aniDirPath, Types type);
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
	int getDamage() const override;

private:
	enum class State : int8_t { Fly, Wait, Explos };
	int32_t _delayBeforeExplos; // in milliseconds
	State _state;
};


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

class TProjectile : public Projectile
{
public:
	TProjectile(shared_ptr<Animation> ani, int damage, D2D1_POINT_2F speed, D2D1_POINT_2F initialPosition);
};


bool isProjectile(BasePlaneObject* obj);
bool isClawProjectile(BasePlaneObject* obj);
bool isEnemyProjectile(BasePlaneObject* obj);
