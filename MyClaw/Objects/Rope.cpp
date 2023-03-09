#include "Rope.h"
#include "../AssetsManager.h"
#include "../Player.h"


#define RECT_OFFSET 15


D2D1_POINT_2F Rope::getRopeEdgePos()
{
	// based on OpenClaw
	static D2D1_POINT_2F ropeFrameIndexToRopeHandleOffset[] = {
		{ -173, 22 }, { -172, 23 }, { -172, 24 }, { -170, 27 }, { -169, 29 },
		{ -168, 31 }, { -166, 35 }, { -164, 39 }, { -163, 44 }, { -162, 47 },
		{ -161, 52 }, { -155, 57 }, { -149, 63 }, { -144, 71 }, { -139, 76 },
		{ -132, 83 }, { -121, 88 }, { -111, 93 }, { -100, 98 }, { -89, 103 },
		{ -78, 109 }, { -66, 111 }, { -54, 113 }, { -42, 116 }, { -30, 118 },
		{ -18, 120 }, { -5, 120 }, { 8, 120 }, { 20, 120 }, { 33, 119 },
		{ 45, 119 }, { 58, 116 }, { 71, 113 }, { 84, 110 }, { 96, 107 },
		{ 108, 102 }, { 118, 98 }, { 127, 92 }, { 136, 86 }, { 145, 81 },
		{ 153, 75 }, { 158, 70 }, { 163, 64 }, { 168, 58 }, { 172, 53 },
		{ 177, 48 }, { 178, 46 }, { 179, 44 }, { 180, 43 }, { 180, 41 },
		{ 180, 40 }, { 180, 38 }, { 180, 37 }, { 180, 36 }, { 180, 34 },
		{ 180, 33 }, { 180, 30 }, { 180, 28 }, { 180, 26 }, { 179, 24 },
		{ 179, 22 }, { 177, 25 }, { 176, 26 }, { 175, 29 }, { 173, 31 },
		{ 171, 33 }, { 170, 37 }, { 168, 41 }, { 167, 44 }, { 166, 49 },
		{ 165, 53 }, { 159, 59 }, { 153, 65 }, { 148, 71 }, { 142, 77 },
		{ 136, 83 }, { 124, 88 }, { 114, 93 }, { 103, 98 }, { 92, 103 },
		{ 81, 108 }, { 69, 110 }, { 56, 112 }, { 44, 114 }, { 33, 117 },
		{ 21, 119 }, { 8, 119 }, { -5, 119 }, { -18, 118 }, { -29, 118 },
		{ -42, 118 }, { -54, 117 }, { -67, 111 }, { -79, 108 }, { -91, 105 },
		{ -104, 101 }, { -113, 96 }, { -122, 91 }, { -131, 85 }, { -140, 79 },
		{ -149, 74 }, { -153, 69 }, { -158, 63 }, { -163, 58 }, { -168, 52 },
		{ -173, 47 }, { -173, 46 }, { -174, 44 }, { -175, 43 }, { -176, 40 },
		{ -176, 39 }, { -176, 37 }, { -176, 36 }, { -176, 34 }, { -176, 33 },
		{ -176, 32 }, { -176, 30 }, { -176, 27 }, { -174, 25 }, { -173, 23 },
	};

	size_t idx = _ani->getFrameNumber();

	return {
		position.x + ropeFrameIndexToRopeHandleOffset[idx].x,
		position.y + ropeFrameIndexToRopeHandleOffset[idx].y
	};
}


Rope::Rope(const WwdObject& obj, Player* player)
	: BasePlaneObject(obj, player)
{
	int32_t speed = obj.speedX;
	if (speed == 0)
	{
		speed = 1500;
	}

	//speed *= 2;
	speed += speed;
	speed /= 120;

	_ani = AssetsManager::createCopyAnimationFromDirectory(PathManager::getImageSetPath(obj.imageSet), speed, false);
	_ani->position = position;
}

void Rope::Logic(uint32_t elapsedTime)
{
	D2D1_RECT_F thisRc = GetRect();

	if (!_player->isJumping() && CollisionDistances::isCollision(_player->GetRect(), thisRc))
	{
		_player->rope = this;
	}

	_ani->Logic(elapsedTime);

	if (_player->rope == this)
	{
		_player->position.x = thisRc.left + RECT_OFFSET;
		_player->position.y = thisRc.top + RECT_OFFSET;
	}
}
void Rope::Draw()
{
	_ani->Draw();
}
D2D1_RECT_F Rope::GetRect()
{
	D2D1_POINT_2F edgePos = getRopeEdgePos();
	return {
		edgePos.x - RECT_OFFSET,
		edgePos.y - RECT_OFFSET,
		edgePos.x + RECT_OFFSET,
		edgePos.y + RECT_OFFSET
	};
}

bool Rope::isPassedHalf() const
{
	return _ani->isPassedHalf();
}
