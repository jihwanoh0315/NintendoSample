#include "Circle.h"

Circle::Circle() : x(0), y(0), r(0)
{
}

Circle::Circle(float x_, float y_, float r_) : x(x_), y(y_), r(r_)
{

}

Circle::~Circle()
{
}

bool Circle::Collide(Circle other_)
{
	float xGap = x - other_.x;
	float yGap = y - other_.y;
	float rSum = r + other_.r;

	if (((xGap * xGap) + (yGap * yGap)) <= (rSum * rSum))
	{
		return true;
	}

	return false;
}

bool Circle::inCircle(float x_, float y_)
{
	float xGap = x - x_;
	float yGap = y - y_;

	if (((xGap * xGap) + (yGap * yGap)) <= r * r)
	{
		return true;
	}

	return false;
}
