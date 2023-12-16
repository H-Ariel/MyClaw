#include "Rope.h"
#include "../Assets-Managers/AssetsManager.h"
#include "../Player.h"


#define RECT_OFFSET 5


// based on OpenClaw
static D2D1_POINT_2F RopeFrameIndexToRopeHandleOffset[120] = {
	{ -173,  22 }, { -172,  23 }, { -172,  24 }, { -170,  27 }, { -169,  29 },
	{ -168,  31 }, { -166,  35 }, { -164,  39 }, { -163,  44 }, { -162,  47 },
	{ -161,  52 }, { -155,  57 }, { -149,  63 }, { -144,  71 }, { -139,  76 },
	{ -132,  83 }, { -121,  88 }, { -111,  93 }, { -100,  98 }, {  -89, 103 },
	{  -78, 109 }, {  -66, 111 }, {  -54, 113 }, {  -42, 116 }, {  -30, 118 },
	{  -18, 120 }, {   -5, 120 }, {    8, 120 }, {   20, 120 }, {   33, 119 },
	{   45, 119 }, {   58, 116 }, {   71, 113 }, {   84, 110 }, {   96, 107 },
	{  108, 102 }, {  118,  98 }, {  127,  92 }, {  136,  86 }, {  145,  81 },
	{  153,  75 }, {  158,  70 }, {  163,  64 }, {  168,  58 }, {  172,  53 },
	{  177,  48 }, {  178,  46 }, {  179,  44 }, {  180,  43 }, {  180,  41 },
	{  180,  40 }, {  180,  38 }, {  180,  37 }, {  180,  36 }, {  180,  34 },
	{  180,  33 }, {  180,  30 }, {  180,  28 }, {  180,  26 }, {  179,  24 },
	{  179,  22 }, {  177,  25 }, {  176,  26 }, {  175,  29 }, {  173,  31 },
	{  171,  33 }, {  170,  37 }, {  168,  41 }, {  167,  44 }, {  166,  49 },
	{  165,  53 }, {  159,  59 }, {  153,  65 }, {  148,  71 }, {  142,  77 },
	{  136,  83 }, {  124,  88 }, {  114,  93 }, {  103,  98 }, {   92, 103 },
	{   81, 108 }, {   69, 110 }, {   56, 112 }, {   44, 114 }, {   33, 117 },
	{   21, 119 }, {    8, 119 }, {   -5, 119 }, {  -18, 118 }, {  -29, 118 },
	{  -42, 118 }, {  -54, 117 }, {  -67, 111 }, {  -79, 108 }, {  -91, 105 },
	{ -104, 101 }, { -113,  96 }, { -122,  91 }, { -131,  85 }, { -140,  79 },
	{ -149,  74 }, { -153,  69 }, { -158,  63 }, { -163,  58 }, { -168,  52 },
	{ -173,  47 }, { -173,  46 }, { -174,  44 }, { -175,  43 }, { -176,  40 },
	{ -176,  39 }, { -176,  37 }, { -176,  36 }, { -176,  34 }, { -176,  33 },
	{ -176,  32 }, { -176,  30 }, { -176,  27 }, { -174,  25 }, { -173,  23 },
};


Rope::Rope(const WwdObject& obj)
	: BasePlaneObject(obj), _edgePos({})
{
	myMemCpy(logicZ, DefaultZCoord::Characters + 1);
	_ani = AssetsManager::createAnimationFromDirectory(PathManager::getImageSetPath(obj.imageSet), false, obj.speedX ? (obj.speedX / 60) : 25);
}

void Rope::Logic(uint32_t elapsedTime)
{
	D2D1_POINT_2F offset = RopeFrameIndexToRopeHandleOffset[_ani->getFrameNumber()];
	_edgePos.x = position.x + offset.x;
	_edgePos.y = position.y + offset.y;

	if (!player->isJumping() && player->GetRect().intersects(GetRect()))
	{
		player->rope = this;
	}

	if (player->rope == this)
	{
		player->position.x = _edgePos.x;
		player->position.y = _edgePos.y;

		if (player->isMirrored())
		{
			player->position.x -= 2 * RECT_OFFSET;
		}
		else
		{
			player->position.x += 2 * RECT_OFFSET;
		}
	}
}
Rectangle2D Rope::GetRect()
{
	return Rectangle2D(
		_edgePos.x - RECT_OFFSET,
		_edgePos.y - RECT_OFFSET,
		_edgePos.x + RECT_OFFSET,
		_edgePos.y + RECT_OFFSET
	);
}
