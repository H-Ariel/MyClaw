#pragma once

#include "Projectile.h"


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
	
	bool isStartExplode() const;

private:
	enum class State : int8_t { Fly, Wait, Explos };
	int _delayBeforeExplos; // in milliseconds
	State _state;
	bool _isPlaySound;
};
