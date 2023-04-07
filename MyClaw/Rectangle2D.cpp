#include "Rectangle2D.h"


Rectangle2D::Rectangle2D()
{
	left = 0;
	top = 0;
	right = 0;
	bottom = 0;
}
Rectangle2D::Rectangle2D(D2D1_RECT_F rect)
{
	left = rect.left;
	top = rect.top;
	right = rect.right;
	bottom = rect.bottom;
}
Rectangle2D::Rectangle2D(float left, float top, float right, float bottom)
{
	this->left = left;
	this->top = top;
	this->right = right;
	this->bottom = bottom;
}

Rectangle2D Rectangle2D::getCollision(const Rectangle2D& other) const
{
	D2D1_RECT_F collision = {};

	if (top < other.bottom && bottom > other.top && left < other.right && right > other.left)
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
	if (top != 0 && top < bottom && top < left && top < right)
	{
		bottom = 0;
		left = 0;
		right = 0;
	}
	else if (bottom != 0 && bottom < top && bottom < left && bottom < right)
	{
		top = 0;
		left = 0;
		right = 0;
	}
	else if (left != 0 && left < top && left < bottom && left < right)
	{
		top = 0;
		bottom = 0;
		right = 0;
	}
	else if (right != 0 && right < top && right < bottom && right < left)
	{
		top = 0;
		bottom = 0;
		left = 0;
	}
}
void Rectangle2D::keepLargest()
{
	if (top != 0 && top > bottom && top > left && top > right)
	{
		bottom = 0;
		left = 0;
		right = 0;
	}
	else if (bottom != 0 && bottom > top && bottom > left && bottom > right)
	{
		top = 0;
		left = 0;
		right = 0;
	}
	else if (left != 0 && left > top && left > bottom && left > right)
	{
		top = 0;
		bottom = 0;
		right = 0;
	}
	else if (right != 0 && right > top && right > bottom && right > left)
	{
		top = 0;
		bottom = 0;
		left = 0;
	}
}

bool Rectangle2D::operator==(const Rectangle2D& other) const
{
	return left == other.left && top == other.top && right == other.right && bottom == other.bottom;
}
bool Rectangle2D::operator!=(const Rectangle2D& other) const
{
	return !(*this == other);
}
