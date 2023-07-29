#include "BasePlaneObject.h"
#include "Player.h"


Player* BasePlaneObject::player = nullptr;

BasePlaneObject::BasePlaneObject(const WwdObject& obj)
	: UIBaseElement({ (float)obj.x, (float)obj.y }),
	ZCoord(obj.z), removeObject(false), _ani(nullptr),
	_isMirrored(obj.drawFlags & WwdObject::Mirror), // TODO: parse all flags
	_isVisible(!(obj.drawFlags & WwdObject::NoDraw)) {}
void BasePlaneObject::Draw()
{
	// TODO: some object need only `_ani->Draw();`, so we need create 2 functions...

	if (_isVisible && _ani)
	{
		_ani->position = position;
		_ani->mirrored = _isMirrored;
		_ani->updateImageData();
		_ani->Draw();
	}
}
Rectangle2D BasePlaneObject::GetRect()
{
	// TODO: some object need only `_ani->GetRect();`, so we need create 2 functions...

	_ani->position = position;
	_ani->mirrored = _isMirrored;
	_ani->updateImageData();
	return _ani->GetRect();
}
void BasePlaneObject::Reset() {}
bool BasePlaneObject::tryCatchPlayer()
{
	if (player->isFalling())
	{
		Rectangle2D colRc = player->GetRect().getCollision(GetRect());
		if ((colRc.right > 0 || colRc.left > 0) && colRc.getSmallest().bottom > 0)
		{
			// if player is falling/going to this object - catch him
			player->stopFalling(colRc.bottom);
			return true;
		}
	}
	return false;
}

BaseStaticPlaneObject::BaseStaticPlaneObject(const WwdObject& obj)
	: BasePlaneObject(obj) {}
void BaseStaticPlaneObject::Logic(uint32_t elapsedTime) {}
Rectangle2D BaseStaticPlaneObject::GetRect() { return _objRc; }
void BaseStaticPlaneObject::setObjectRectangle() { myMemCpy(_objRc, BasePlaneObject::GetRect()); }

BaseDynamicPlaneObject::BaseDynamicPlaneObject(const WwdObject& obj)
	: BasePlaneObject(obj), speed({}) {}
bool BaseDynamicPlaneObject::isFalling() const { return speed.y > 0; }
void BaseDynamicPlaneObject::stopFalling(float collisionSize) { speed.y = 0; position.y -= collisionSize; }
void BaseDynamicPlaneObject::stopMovingLeft(float collisionSize) { speed.x = 0; position.x += collisionSize; }
void BaseDynamicPlaneObject::stopMovingRight(float collisionSize) { speed.x = 0; position.x -= collisionSize; }
void BaseDynamicPlaneObject::bounceTop() { speed.y = abs(speed.y); }

BaseDamageObject::BaseDamageObject(const WwdObject& obj, int damage)
	: BaseStaticPlaneObject(obj), _damage(damage) {}
