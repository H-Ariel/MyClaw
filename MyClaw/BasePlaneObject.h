#pragma once

#include "Animation.h"
#include "WwdFile.h"


class Player;


class BasePlaneObject : public UIBaseElement
{
public:
	BasePlaneObject(const WwdObject& obj, Player* player = nullptr);
	virtual void Logic(uint32_t elapsedTime) override = 0;
	virtual void Draw() override;
	virtual Rectangle2D GetRect() override;
	virtual void Reset(); // this method is called after Captain Claw is dead. It can be used to reset the object

	const uint32_t ZCoord;
	bool removeObject;

protected:
	bool tryCatchPlayer(); // returns if successfully caught the player

	Player* const _player;
	shared_ptr<Animation> _ani;
	bool _isMirrored, _isVisible;
};

// an object that does not move during gameplay
class BaseStaticPlaneObject : public BasePlaneObject
{
public:
	BaseStaticPlaneObject(const WwdObject& obj, Player* player = nullptr);
	void Logic(uint32_t elapsedTime) override;
	Rectangle2D GetRect() override;

protected:
	void setObjectRectangle(); // call this after defination of animation

	const Rectangle2D _objRc;
};


#define GRAVITY 2048e-6f //(2.048f/1000)

// an object that can move during gameplay
class BaseDynamicPlaneObject : public BasePlaneObject
{
public:
	BaseDynamicPlaneObject(const WwdObject& obj, Player* player = nullptr);

	float getSpeedX() const { return _speed.x; }
	float getSpeedY() const { return _speed.y; }
	void setSpeedX(float speedX) { _speed.x = speedX; }
	void setSpeedY(float speedY) { _speed.y = speedY; }

	virtual bool isFalling() const;

	virtual void stopFalling(float collisionSize);
	virtual void stopMovingLeft(float collisionSize);
	virtual void stopMovingRight(float collisionSize);
	virtual void bounceTop();

protected:
	// speed.x -> speed of X-axis, speed.y -> speed of Y-axis
	// the speed units are pixels per milliseconds
	// example for usage: `position.x = speed.x * elapsedTime`
	D2D1_POINT_2F _speed;
};
