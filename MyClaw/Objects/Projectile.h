#pragma once

#include "../BasePlaneObject.h"

#define DEFAULT_PROJECTILE_SPEED 400


class Projectile : public BaseDynamicPlaneObject
{
public:
	Projectile(const WwdObject& obj, const string& aniDirPath, const string& imageSet = "");
	void Logic(uint32_t elapsedTime) override;

	void bounceTop() override;
	void stopFalling(float collisionSize) override;

	bool isClawBullet() const; // pistol shot
	bool isClawDynamite() const;

	virtual int getDamage() const;

protected:
	Projectile(shared_ptr<Animation> ani, int damage, D2D1_POINT_2F speed, D2D1_POINT_2F initialPosition);

private:
	int32_t _timeLeft; // time left before projectile disappears
	const int _damage;
};


bool isProjectile(BasePlaneObject* obj);
bool isClawProjectile(BasePlaneObject* obj);
bool isEnemyProjectile(BasePlaneObject* obj);
