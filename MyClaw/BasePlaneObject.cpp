#include "BasePlaneObject.h"
#include "PhysicsManager.h"
#include "Player.h"


shared_ptr<Player> BasePlaneObject::player;
shared_ptr<PhysicsManager> BasePlaneObject::physics;


BasePlaneObject::BasePlaneObject(const WwdObject& obj)
	: UIBaseElement({ (float)obj.x, (float)obj.y }),
	logicZ(obj.z), drawZ(obj.z), removeObject(false), _ani(nullptr),
	_isMirrored(obj.drawFlags & WwdObject::Mirror),
	_isVisible(!(obj.drawFlags & WwdObject::NoDraw)) {}
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
Rectangle2D BasePlaneObject::GetRect()
{
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
		float smallestBottom = colRc.getSmallest().bottom;
		if ((colRc.right > 0 || colRc.left > 0) && (0 < smallestBottom && smallestBottom < 16))
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
void BaseDynamicPlaneObject::whenTouchDeath() { removeObject = true; }



BaseDamageObject::BaseDamageObject(const WwdObject& obj, int damage)
	: BaseStaticPlaneObject(obj), _damage(damage) {}


BaseSoundObject::BaseSoundObject(const WwdObject& obj)
	: BaseStaticPlaneObject(obj),
	_wavPath(PathManager::getSoundFilePath(obj.animation))
{
	_volume = obj.damage ? obj.damage : 100;

	Rectangle2D newRc;

	if (obj.minX != 0)
	{
		newRc.left = (float)obj.minX;
		newRc.top = (float)obj.minY;
		newRc.right = (float)obj.maxX;
		newRc.bottom = (float)obj.maxY;
	}
	else
	{
		D2D1_SIZE_F size = { 128, 128 };

		if (contains(obj.logic, "Tiny")) size = { 32, 32 };
		else if (contains(obj.logic, "Small")) size = { 64, 64 };
		else if (contains(obj.logic, "Big" )) size = { 256, 256 };
		else if (contains(obj.logic, "Huge")) size = { 512, 512 };
		else if (contains(obj.logic, "Wide")) size = { 200, 64 }; // maybe 256 ?
		else if (contains(obj.logic, "Tall")) size = { 64, 200 };
		//else size = { 128, 128 };

		newRc.left = position.x - size.width / 2;
		newRc.top = position.y - size.height / 2;
		newRc.right = newRc.left + size.width;
		newRc.bottom = newRc.top + size.height;
	}

	myMemCpy(_objRc, newRc);
}
void BaseSoundObject::Draw() {} // these objects are invisible so no need to draw them


OneTimeAnimation::OneTimeAnimation(D2D1_POINT_2F pos, shared_ptr<UIAnimation> ani, bool removeAtEnd)
	: BasePlaneObject({}), _removeAtEnd(removeAtEnd)
{
	drawZ = DefaultZCoord::Characters + 2; // always cover characters
	_ani = ani;
	_ani->loopAni = false;
	_ani->position = pos;
}
OneTimeAnimation::OneTimeAnimation(D2D1_POINT_2F pos, const string& aniPath, const string& imageSet, bool removeAtEnd)
	: OneTimeAnimation(pos, AssetsManager::loadCopyAnimation(
		PathManager::getAnimationPath(aniPath), PathManager::getImageSetPath(imageSet)), removeAtEnd)
{
}
void OneTimeAnimation::Logic(uint32_t elapsedTime)
{
	_ani->Logic(elapsedTime);
	if (_removeAtEnd)
		removeObject = _ani->isFinishAnimation();
}
void OneTimeAnimation::Draw()
{
	_ani->Draw();
}
