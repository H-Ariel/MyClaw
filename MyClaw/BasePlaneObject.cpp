#include "BasePlaneObject.h"
#include "Player.h"


BasePlaneObject::BasePlaneObject(const WwdObject& obj, Player* player)
	: UIBaseElement({ (float)obj.x, (float)obj.y }), _player(player),
	ZCoord(obj.z), removeObject(false), _ani(nullptr),
	_isMirrored(obj.drawFlags& WwdObject::Mirror), // TODO: parse all flags
	_isVisible(!(obj.drawFlags& WwdObject::NoDraw)) {}
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
	Rectangle2D colRc = _player->GetRect().getCollision(GetRect());
	if (colRc.getSmallest().bottom > 0 && (colRc.right > 0 || colRc.left > 0) && _player->isFalling())
	{
		// if the player fall or go to this object - catch him
		_player->stopFalling(colRc.bottom);
		return true;
	}
	return false;
}

BaseStaticPlaneObject::BaseStaticPlaneObject(const WwdObject& obj, Player* player)
	: BasePlaneObject(obj, player) {}
void BaseStaticPlaneObject::Logic(uint32_t elapsedTime) {}
Rectangle2D BaseStaticPlaneObject::GetRect() { return _objRc; }
void BaseStaticPlaneObject::setObjectRectangle() { myMemCpy(_objRc, BasePlaneObject::GetRect()); }

BaseDynamicPlaneObject::BaseDynamicPlaneObject(const WwdObject& obj, Player* player)
	: BasePlaneObject(obj, player), _speed({}) {}
void BaseDynamicPlaneObject::stopFalling(float collisionSize) { _speed.y = 0; position.y -= collisionSize; }
void BaseDynamicPlaneObject::stopMovingLeft(float collisionSize) { _speed.x = 0; position.x += collisionSize; }
void BaseDynamicPlaneObject::stopMovingRight(float collisionSize) { _speed.x = 0; position.x -= collisionSize; }
void BaseDynamicPlaneObject::bounceTop() { _speed.y = abs(_speed.y); }
