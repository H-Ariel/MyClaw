#include "InvincibilityComponent.h"
#include "Player.h"


const vector<ColorF> InvincibilityComponent::colors = {
	ColorF(0, 0, 1, 1),
	ColorF(0, 1, 1, 1),
	ColorF(0, 1, 0, 1),
	ColorF(1, 1, 0, 1),
	ColorF(1, 0, 0, 1),
	ColorF(1, 0, 1, 1)
};


InvincibilityComponent::InvincibilityComponent(Player* player) 
	: player(player), currColorIdx(0), _timeCounter(0) {}


void InvincibilityComponent::update(uint32_t elapsedTime)
{
	_timeCounter += elapsedTime;
	if (_timeCounter >= 150) // change color every 150ms
	{
		_timeCounter = 0;
		calcNextColor();
		player->updateInvincibilityColorEffect();
	}
}
