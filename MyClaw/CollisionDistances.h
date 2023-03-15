#pragma once

#include <d2d1.h> // for D2D1_RECT_F


class CollisionDistances
{
public:
	// save only the smallest value. the other becaome to 0.
	static void keepSmallest(D2D1_RECT_F& rect);

	// save only the largest value. the other becaome to 0.
	static void keepLargest(D2D1_RECT_F& rect);

	static D2D1_RECT_F getSmallest(D2D1_RECT_F rect) { keepSmallest(rect); return rect; }
	static D2D1_RECT_F getLargest(D2D1_RECT_F rect) { keepLargest(rect); return rect; }
	static bool isEmpty(D2D1_RECT_F rect) { return rect.top == 0 && rect.bottom == 0 && rect.right == 0 && rect.left == 0; }

	// if `a` and `b` collide - return the collision distances, else - empty rectangle
	static D2D1_RECT_F getCollision(D2D1_RECT_F a, D2D1_RECT_F b);

	// return if `a` and `b` are collide
	static bool isCollision(D2D1_RECT_F a, D2D1_RECT_F b) { return !isEmpty(getCollision(a, b)); }
};
