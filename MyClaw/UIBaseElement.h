#pragma once

#include "framework.h"


class UIBaseElement
{
public:
	UIBaseElement(D2D1_POINT_2F position = {});
	virtual ~UIBaseElement();

	virtual void Logic(uint32_t elapsedTime); // in milliseconds
	virtual void Draw();

	virtual D2D1_RECT_F GetRect();


	D2D1_POINT_2F position;
};
