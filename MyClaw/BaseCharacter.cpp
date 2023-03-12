#include "BaseCharacter.h"
#include "WindowManager.h"


BaseCharacter::BaseCharacter(const WwdObject& obj, Player* player)
	: BaseDynamicPlaneObject(obj, player), _health(obj.health), _forward(true), 
	_lastAttackRect({}), _saveCurrRect({}), _isAttack(false)
{
}

void BaseCharacter::Draw()
{
	_ani->Draw();
#define DRAW_RECTANGLES
#ifdef DRAW_RECTANGLES
	WindowManager::drawRect(GetRect(), ColorF::Blue);
	WindowManager::drawRect(GetAttackRect().first, ColorF::LightSkyBlue);
#endif
}

bool BaseCharacter::isStanding() const { return false; }
bool BaseCharacter::isDuck() const { return false; }
bool BaseCharacter::isTakeDamage() const { return false; }
