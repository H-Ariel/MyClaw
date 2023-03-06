#include "BasePlaneObject.h"
#include "Player.h"


BasePlaneObject::BasePlaneObject(const WwdObject& obj, Player* player)
	: UIBaseElement({ (float)obj.x, (float)obj.y }), _player(player),
	ZCoord(obj.z), removeObject(false), _ani(nullptr),
	_isMirrored(obj.drawFlags& WwdObject::Mirror), // TODO: parse all flags
	_isVisible(!(obj.drawFlags& WwdObject::NoDraw)) {}
void BasePlaneObject::Draw()
{
	if (_isVisible && _ani)
	{
		_ani->position = position;
		_ani->mirrored = _isMirrored;
		_ani->updateImageData();
		_ani->Draw();
	}
}
D2D1_RECT_F BasePlaneObject::GetRect()
{
	_ani->position = position;
	_ani->mirrored = _isMirrored;
	_ani->updateImageData();
	return _ani->GetRect();
}
void BasePlaneObject::Reset() {}
bool BasePlaneObject::tryCatchPlayer()
{
	const D2D1_RECT_F colRc = CollisionDistances::getCollision(_player->GetRect(), GetRect());
	D2D1_RECT_F smallest(colRc);
	CollisionDistances::keepSmallest(smallest);
	if (smallest.bottom > 0 && (colRc.right > 0 || colRc.left > 0) && _player->isFalling())
	{
		// if the player fall or go to this object - catch him
		_player->stopFalling(colRc.bottom);
		return true;
	}
	return false;
}

BaseStaticPlaneObject::BaseStaticPlaneObject(const WwdObject& obj, Player* player)
	: BasePlaneObject(obj, player), _objRc({}) {}
void BaseStaticPlaneObject::Logic(uint32_t elapsedTime) {}
D2D1_RECT_F BaseStaticPlaneObject::GetRect() { return _objRc; }
void BaseStaticPlaneObject::setObjectRectangle() { myMemCpy(_objRc, BasePlaneObject::GetRect()); }

BaseDynamicPlaneObject::BaseDynamicPlaneObject(const WwdObject& obj, Player* player)
	: BasePlaneObject(obj, player), _speed({}) {}
void BaseDynamicPlaneObject::stopFalling(float collisionSize) { _speed.y = 0; position.y -= collisionSize; }
void BaseDynamicPlaneObject::stopMovingLeft(float collisionSize) { _speed.x = 0; position.x += collisionSize; }
void BaseDynamicPlaneObject::stopMovingRight(float collisionSize) { _speed.x = 0; position.x -= collisionSize; }
void BaseDynamicPlaneObject::bounceTop() { _speed.y = abs(_speed.y); }
