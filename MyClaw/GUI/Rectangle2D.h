#pragma once

#include <d2d1.h> // D2D1_RECT_F

// this class is a wrapper for D2D1_RECT_F
class Rectangle2D : public D2D1_RECT_F
{
public:
	Rectangle2D();
	Rectangle2D(D2D1_RECT_F rect);
	Rectangle2D(float left, float top, float right, float bottom);

	Rectangle2D getCollision(const Rectangle2D& other) const;
	void keepSmallest(); // save only the smallest value. the other becaome to 0.
	void keepLargest(); // save only the largest value. the other becaome to 0.

	bool intersects(const Rectangle2D& rect) const { return !getCollision(rect).isEmpty(); } // return if this and `rect` are collide
	Rectangle2D getSmallest() const { Rectangle2D copy(*this); copy.keepSmallest(); return copy; }
	Rectangle2D getLargest() const { Rectangle2D copy(*this); copy.keepLargest(); return copy; }
	bool isEmpty() const { return top == 0 && bottom == 0 && right == 0 && left == 0; }

	bool operator==(const Rectangle2D& other) const;
	bool operator!=(const Rectangle2D& other) const;
};
