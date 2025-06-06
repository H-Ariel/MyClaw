#pragma once

#include "../BasePlaneObject.h"

constexpr int DEFAULT_PROJECTILE_SPEED = 400;


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
	Projectile(shared_ptr<UIAnimation> ani, int damage, D2D1_POINT_2F speed, D2D1_POINT_2F initialPosition);

private:
	int _timeLeft; // time left before projectile disappears
	const int _damage;
};
