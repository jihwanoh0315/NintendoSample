#include "Rectangle.h"

Rectangle::Rectangle() : x1(0.0f), y1(0.0f), x2(0.0f), y2(0.0f)
{
}

Rectangle::~Rectangle()
{
}

Rectangle::Rectangle(float x1_, float y1_, float x2_, float y2_) : x1(x1_), y1(y1_), x2(x2_), y2(y2_)
{
}

bool Rectangle::inRect(float x_, float y_)
{
	if (x1 < x_ && x_ < x2
		&& y1 < y_ && y_ < y2)
	{
		return true;
	}

	return false;
}
