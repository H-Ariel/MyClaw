#include "UIBaseImage.h"
#include "WindowManager.h"


UIBaseImage::UIBaseImage(ID2D1Bitmap* bitmap, D2D1_POINT_2F offset)
	: _bitmap(bitmap), offset(offset), size(bitmap->GetSize()), mirrored(false)
{
}

void UIBaseImage::Draw()
{
	WindowManager::drawBitmap(_bitmap, GetRect(), mirrored);
}

D2D1_RECT_F UIBaseImage::GetRect()
{
	D2D1_RECT_F rc = {};
	rc.left = position.x - size.width / 2;
	if (mirrored) rc.left -= offset.x;
	else rc.left += offset.x;
	rc.top = position.y - size.height / 2 + offset.y;
	rc.right = rc.left + size.width;
	rc.bottom = rc.top + size.height;
	return rc;
}

shared_ptr<UIBaseImage> UIBaseImage::getCopy()
{
	shared_ptr<UIBaseImage> newImg(DBG_NEW UIBaseImage(_bitmap, offset));
	newImg->mirrored = mirrored;
	return newImg;
}
