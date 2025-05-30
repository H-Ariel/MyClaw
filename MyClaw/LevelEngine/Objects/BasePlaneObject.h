#pragma once

#include "Assets-Managers/AssetsManager.h"


constexpr float GRAVITY = 2048e-6f; // gravity acceleration (2.048f/1000)
constexpr float MAX_Y_SPEED = 1.500f; // max vertical speed


// the base class for all objects that are used in the game
class BasePlaneObject : public UIBaseElement
{
public:
	enum DefaultZCoord
	{
		BackCrate = 1000, BackCandy = 1000, CheckpointFlag = 1000,
		Elevators = 2000, Pegs = 2000,
		Items = 3000,
		Characters = 4000,
		FrontCrate = 5000, FrontCandy = 5000,
		Columns = 7000, Structures = 7000,
		Coverups = 8000,
		EnemyFallDeath = 8500,
		Interface = 9000
	};

	BasePlaneObject(const WwdObject& obj);
	virtual void Logic(uint32_t elapsedTime) override = 0;
	virtual void Draw() override;
	virtual Rectangle2D GetRect() override;
	virtual void Reset(); // this method is called after Captain Claw is dead. It can be used to reset the object
	virtual void enterEasyMode(); // thats a cheat-code!
	virtual void exitEasyMode();  // ^^^

	void Delay(int timeDelay) { _timeDelay = timeDelay; } // delay object for `timeDelay` ms
	bool isDelayed() const { return _timeDelay > 0; }
	void decreaseTimeDelay(int ms) { _timeDelay -= ms; }

	bool tryCatchPlayer(); // returns if successfully caught the player
	bool isMirrored() const { return _isMirrored; }

	int logicZ, drawZ;
	bool removeObject;

protected:
	shared_ptr<UIAnimation> _ani;
	bool _isMirrored, _isVisible, _upsideDown;

private:
	int _timeDelay; // time to dealy in milliseconds. `private` to force use of `Delay`, `isDelayed`, and `decreaseTimeDelay`
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
	virtual void whenTouchDeath(); // when the object touch tile with `TileAttribute_Death` flag

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
	int _volume;
};
