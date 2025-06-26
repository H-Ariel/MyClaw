#include "PowerupSparkles.h"
#include "AssetsManagers/AssetsManager.h"


PowerupSparkles::PowerupSparkles(Rectangle2D* playerRc)
	: _playerRc(playerRc)
{
	for (auto& s : _sparkles)
	{
		s = AssetsManager::loadCopyAnimation("GAME/ANIS/GLITTER1.ANI");
		init(s);
	}
}
void PowerupSparkles::init(shared_ptr<UIAnimation> sparkle)
{
	const float a = (_playerRc->right - _playerRc->left) / 2; // vertical radius
	const float b = (_playerRc->bottom - _playerRc->top) / 2; // horizontal radius
	float x, y;

	do {
		x = getRandomFloat(-a, a);
		y = getRandomFloat(-b, b);
		// check if (x,y) is in the player ellipse
	} while (pow(x / a, 2) + pow(y / b, 2) > 1);

	sparkle->position.x = x + (_playerRc->right + _playerRc->left) / 2;
	sparkle->position.y = y + (_playerRc->bottom + _playerRc->top) / 2;

	for (int i = 0, n = getRandomInt(0, 3); i < n; i++) sparkle->Logic(50);
}
void PowerupSparkles::Logic(uint32_t elapsedTime)
{
	for (auto& s : _sparkles)
	{
		if (s->isFinishAnimation())
			init(s);
		s->Logic(elapsedTime);
	}
}
void PowerupSparkles::Draw()
{
	for (auto& s : _sparkles) s->Draw();
}
