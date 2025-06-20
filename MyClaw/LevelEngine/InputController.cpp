#include "InputController.h"
#include "Objects/Player/Player.h"
#include "GlobalObjects.h"
#include "CheatsManager.h"


InputController::InputController(Player* player) : player(player) {}

void InputController::keyUp(int key)
{
	GO::cheats->addKey(key);

	// check if player can do something
	if (player->isInDeathAnimation() || player->_freezeTime > 0) return;
	
	// manage player actions
	switch (key)
	{
	case VK_UP:		player->_upPressed = false; break;
	case VK_DOWN:	player->_downPressed = false; break;
	case VK_LEFT:	player->_leftPressed = false; break;
	case VK_RIGHT:	player->_rightPressed = false; break;
	case 'Z':		player->onZPressed(); break;

	case VK_SHIFT:	if (!player->_useWeapon) player->_inventory.setNextWeapon(); break;
	case '1':		if (!player->_useWeapon) player->_inventory.setWeapon(ClawProjectile::Types::Pistol); break;
	case '2':		if (!player->_useWeapon) player->_inventory.setWeapon(ClawProjectile::Types::Magic); break;
	case '3':		if (!player->_useWeapon) player->_inventory.setWeapon(ClawProjectile::Types::Dynamite); break;

	case VK_MENU:		player->_useWeapon = !player->_isOnLadder && !player->rope && !player->_raisedPowderKeg; player->_altPressed = false; break;
	case VK_CONTROL:	player->_isAttack = !player->_isOnLadder && !player->_altPressed && !player->rope && !player->_raisedPowderKeg; break;
	}
}
void InputController::keyDown(int key)
{
	// check if player can do something
	if (player->isInDeathAnimation() || player->_useWeapon || player->_freezeTime > 0) return;

	// manage player actions
	switch (key)
	{
	case VK_UP:		if (!player->_useWeapon && !player->_isAttack && !player->_altPressed && !player->rope && !player->_raisedPowderKeg) player->_upPressed = true; break;
	case VK_DOWN:	if (!player->rope && !player->_raisedPowderKeg) player->_downPressed = true; break;
	case VK_LEFT:	player->_leftPressed = true; break;
	case VK_RIGHT:	player->_rightPressed = true; break;
	case VK_SPACE:	player->onSpacePressed(); break;
	case VK_MENU:	if (!player->rope && !player->_raisedPowderKeg) {
		if (!player->_altPressed) player->_holdAltTime = 0;
		player->_altPressed = (!player->_inventory.hasDynamiteEquipped() || player->_inventory.getWeaponAmount(ClawProjectile::Types::Dynamite) > 0);
	} break;
	}
}
