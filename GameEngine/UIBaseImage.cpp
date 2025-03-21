#include "UIBaseImage.h"
#include "WindowManager.h"


UIBaseImage::UIBaseImage(ID2D1Bitmap* bitmap, D2D1_POINT_2F offset)
	: _bitmap(bitmap), offset(offset), size({}), opacity(1.0f), mirrored(false), upsideDown(false)
{
	if (_bitmap)
		size = _bitmap->GetSize();
}

void UIBaseImage::Draw()
{
	WindowManager::drawImage(this);
}

Rectangle2D UIBaseImage::GetRect()
{
	Rectangle2D rc;
	rc.left = position.x - size.width / 2;
	if (mirrored) rc.left -= offset.x;
	else rc.left += offset.x;
	rc.top = position.y - size.height / 2 + offset.y;
	rc.right = rc.left + size.width;
	rc.bottom = rc.top + size.height;
	return rc;
}

// maybe we can delete this function because it is a shallow copy
shared_ptr<UIBaseImage> UIBaseImage::getCopy()
{
	return make_shared<UIBaseImage>(*this);
}
