#pragma once

#include <d2d1.h> // D2D1_RECT_F

// this class is a wrapper for D2D1_RECT_F
class Rectangle2D : public D2D1_RECT_F
{
public:
	Rectangle2D()
		: D2D1_RECT_F{ 0, 0, 0, 0 } {}
	Rectangle2D(D2D1_RECT_F rect)
		: D2D1_RECT_F{ rect.left, rect.top, rect.right, rect.bottom } {}
	Rectangle2D(float left, float top, float right, float bottom)
		: D2D1_RECT_F{ left, top, right, bottom } {}

	bool intersects(const Rectangle2D& rect) const; // returns if this and `rect` are collide
	Rectangle2D getCollision(const Rectangle2D& other) const; // return collision. if the rectangles are not colide returns empty rectangle.
	
	void keepSmallest(); // save only the smallest value. the other becaome to 0.
	void keepLargest();  // save only the largest value. the other becaome to 0.

	Rectangle2D getSmallest() const;
	Rectangle2D getLargest() const;
	bool isEmpty() const { return top == 0 && bottom == 0 && right == 0 && left == 0; }

	bool operator==(const Rectangle2D& other) const;
	bool operator!=(const Rectangle2D& other) const;
};
