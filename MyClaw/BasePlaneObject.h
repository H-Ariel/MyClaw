#pragma once

#include "Animation.h"
#include "Assets-Managers/WwdFile.h"

#define GRAVITY 2048e-6f //(2.048f/1000)

class Player;


// the base class for all objects that are used in the game
class BasePlaneObject : public UIBaseElement
{
public:
	BasePlaneObject(const WwdObject& obj);
	virtual void Logic(uint32_t elapsedTime) override = 0;
	virtual void Draw() override;
	virtual Rectangle2D GetRect() override;
	virtual void Reset(); // this method is called after Captain Claw is dead. It can be used to reset the object

	bool isMirrored() const { return _isMirrored; }

	const uint32_t ZCoord;
	bool removeObject;

	static Player* player;

protected:
	bool tryCatchPlayer(); // returns if successfully caught the player

	shared_ptr<Animation> _ani;
	bool _isMirrored, _isVisible;
};


// an object that does not move during gameplay
class BaseStaticPlaneObject : public BasePlaneObject
{
public:
	BaseStaticPlaneObject(const WwdObject& obj);
	void Logic(uint32_t elapsedTime) override;
	Rectangle2D GetRect() override;

protected:
	void setObjectRectangle(); // call this after defination of animation

	const Rectangle2D _objRc;
};


// an object that can move during gameplay
class BaseDynamicPlaneObject : public BasePlaneObject
{
public:
	BaseDynamicPlaneObject(const WwdObject& obj);

	virtual bool isFalling() const;

	virtual void stopFalling(float collisionSize);
	virtual void stopMovingLeft(float collisionSize);
	virtual void stopMovingRight(float collisionSize);
	virtual void bounceTop();

	// speed.x -> speed of X-axis, speed.y -> speed of Y-axis
	// the speed units are pixels per milliseconds
	// example for usage: `position.x = speed.x * elapsedTime`
	D2D1_POINT_2F speed;
};


// an object that can damage the player`
class BaseDamageObject : public BaseStaticPlaneObject 
{
public:
	BaseDamageObject(const WwdObject& obj, int damage);

	virtual bool isDamage() const = 0;

	int getDamage() const { return isDamage() ? _damage : 0; }

protected:
	const int _damage;
};


class BaseSoundObject : public BaseStaticPlaneObject
{
public:
	BaseSoundObject(const WwdObject& obj);

	void Draw() override;

protected:
	const string _wavPath; // the sound file
	int32_t _volume;
	uint32_t _wavPlayerId;
};
