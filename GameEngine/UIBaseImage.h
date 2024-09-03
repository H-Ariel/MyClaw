#pragma once

#include "UIBaseElement.h"


class UIBaseImage : public UIBaseElement
{
public:
	UIBaseImage(ID2D1Bitmap* bitmap, D2D1_POINT_2F offset = {});

	void Draw() override;
	Rectangle2D GetRect() override;

	shared_ptr<UIBaseImage> getCopy();

	
	D2D1_SIZE_F size;
	float opacity; // value between 0 (transparent) and 1 (opaque)
	bool mirrored; // flag to set whether to draw normal or inverted
	bool upsideDown; // flag to set whether to draw normal or upside-down

private:
	const D2D1_POINT_2F offset;
	ID2D1Bitmap* const _bitmap;

	friend class WindowManager;
};
