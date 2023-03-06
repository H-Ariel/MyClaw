#pragma once

#include <d2d1.h>


namespace CollisionDistances
{
	void keepSmallest(D2D1_RECT_F& rect);
	void keepLargest(D2D1_RECT_F& rect);
	bool isEmpty(D2D1_RECT_F rect);
	
	D2D1_RECT_F getCollision(D2D1_RECT_F a, D2D1_RECT_F b); // if `a` and `b` collide - return the collision distances, else - empty rectangle
	
	bool isCollision(D2D1_RECT_F a, D2D1_RECT_F b); // return if `a` and `b` are collide
	bool isTopCollision(D2D1_RECT_F collisionRect);
	bool isBottomCollision(D2D1_RECT_F collisionRect);
};
