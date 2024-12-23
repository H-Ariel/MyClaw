#pragma once

#include "GameEngineFramework.hpp"


class UIBaseElement
{
public:
	UIBaseElement(D2D1_POINT_2F position = {});
	virtual ~UIBaseElement();

	virtual void Logic(uint32_t elapsedTime); // in milliseconds
	virtual void Draw();

	virtual Rectangle2D GetRect();


	D2D1_POINT_2F position;
};
