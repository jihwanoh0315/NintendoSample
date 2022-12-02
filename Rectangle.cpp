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

void Rectangle::Update()
{
}

void Rectangle::SetPosition(float x_, float y_)
{
	float xHalf = (x2 - x1) / 2.0f;
	float yHalf = (y2 + y1) / 2.0f;

	x1 = x_ - xHalf;
	x2 = x_ + xHalf;
	y1 = y_ - yHalf;
	y2 = y_ + yHalf;
}

void Rectangle::SetSize(float x_, float y_)
{
	float xCenter = (x2 + x1) / 2.0f;
	float yCenter = (y2 + y1) / 2.0f;
	float xHalf = x_ / 2.0f;
	float yHalf = y_  / 2.0f;

	x1 = xCenter - xHalf;
	x2 = xCenter + xHalf;
	y1 = yCenter - yHalf;
	y2 = yCenter + yHalf;
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

void Rectangle::RenderInit()
{
}

void Rectangle::Render()
{

}
