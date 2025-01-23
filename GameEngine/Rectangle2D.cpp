#include "Rectangle2D.h"


bool Rectangle2D::intersects(const Rectangle2D& other) const {
	return top < other.bottom
		&& bottom > other.top
		&& left < other.right
		&& right > other.left;
}
Rectangle2D Rectangle2D::getCollision(const Rectangle2D& other) const
{
	Rectangle2D collision = {};

	if (this->intersects(other))
	{
		collision.top = other.bottom - top;
		collision.bottom = bottom - other.top;
		collision.left = other.right - left;
		collision.right = right - other.left;
	}

	return collision;
}
void Rectangle2D::keepSmallest()
{
	if (top < bottom && top < left && top < right)
	{
		bottom = 0;
		left = 0;
		right = 0;
	}
	else if (bottom < top && bottom < left && bottom < right)
	{
		top = 0;
		left = 0;
		right = 0;
	}
	else if (left < top && left < bottom && left < right)
	{
		top = 0;
		bottom = 0;
		right = 0;
	}
	else if (right < top && right < bottom && right < left)
	{
		top = 0;
		bottom = 0;
		left = 0;
	}
}
void Rectangle2D::keepLargest()
{
	if (top > bottom && top > left && top > right)
	{
		bottom = 0;
		left = 0;
		right = 0;
	}
	else if (bottom > top && bottom > left && bottom > right)
	{
		top = 0;
		left = 0;
		right = 0;
	}
	else if (left > top && left > bottom && left > right)
	{
		top = 0;
		bottom = 0;
		right = 0;
	}
	else if (right > top && right > bottom && right > left)
	{
		top = 0;
		bottom = 0;
		left = 0;
	}
}

Rectangle2D Rectangle2D::getSmallest() const {
	Rectangle2D copy(*this);
	copy.keepSmallest();
	return copy;
}
Rectangle2D Rectangle2D::getLargest() const {
	Rectangle2D copy(*this);
	copy.keepLargest();
	return copy;
}

bool Rectangle2D::operator==(const Rectangle2D& other) const
{
	return left == other.left && top == other.top && right == other.right && bottom == other.bottom;
}
bool Rectangle2D::operator!=(const Rectangle2D& other) const
{
	return !(*this == other);
}
