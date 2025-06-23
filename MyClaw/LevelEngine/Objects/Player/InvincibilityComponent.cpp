#include "InvincibilityComponent.h"
#include "Player.h"
#include "../../GlobalObjects.h"


const vector<ColorF> InvincibilityComponent::colors = {
	ColorF(0, 0, 1, 1),
	ColorF(0, 1, 1, 1),
	ColorF(0, 1, 0, 1),
	ColorF(1, 1, 0, 1),
	ColorF(1, 0, 0, 1),
	ColorF(1, 0, 1, 1)
};

constexpr int CHANGE_COLOR_TIME = 150;

InvincibilityComponent::InvincibilityComponent(Player* player)
	: player(player), currColorIdx(0),
	timer(bind(&InvincibilityComponent::changePlayerColor, this))
{
}

void InvincibilityComponent::changePlayerColor()
{
	currColorIdx = (currColorIdx + 1) % colors.size();  // calc next color
	player->updateInvincibilityColorEffect();
	timer.reset(CHANGE_COLOR_TIME);
}

void InvincibilityComponent::init()
{
	currColorIdx = -1;
	changePlayerColor();
	GO::addTimer(&timer);
}
