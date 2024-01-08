#include "BaseCharacter.h"
#include "GameEngine/WindowManager.h"


BaseCharacter::BaseCharacter(const WwdObject& obj)
	: BaseDynamicPlaneObject(obj), _health(obj.health), _isAttack(false)
{
	logicZ = DefaultZCoord::Characters;
	drawZ = DefaultZCoord::Characters;
}

void BaseCharacter::Draw()
{
	_ani->Draw();
#ifdef _DEBUG
	WindowManager::drawRect(GetRect(), ColorF::Blue);
	if (isAttack())
		WindowManager::drawRect(GetAttackRect().first, ColorF::LightSkyBlue);
#endif
}

bool BaseCharacter::isStanding() const { return false; }
bool BaseCharacter::isDuck() const { return false; }
bool BaseCharacter::isTakeDamage() const { return false; }
bool BaseCharacter::isAttack() const { return _isAttack; }

Rectangle2D BaseCharacter::setRectByCenter(Rectangle2D rc, Rectangle2D rcCenter) const
{
	float addX = position.x - (rcCenter.right - rcCenter.left) / 2;
	float addY = position.y - (rcCenter.bottom - rcCenter.top) / 2;
	rc.top += addY;
	rc.bottom += addY;
	rc.left += addX;
	rc.right += addX;
	return rc;
}
Rectangle2D BaseCharacter::setRectByCenter(Rectangle2D rc) const
{
	return setRectByCenter(rc, rc);
}
