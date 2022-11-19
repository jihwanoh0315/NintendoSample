#pragma once

class Circle
{
public:
	Circle();
	Circle(float x_, float y_, float r_);
	~Circle();

	bool Collide(Circle other_);
	bool inCircle(float x_, float y_);

	float x;
	float y;
	float r;
private:

};


