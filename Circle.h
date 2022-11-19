#pragma once

class Circle
{
public:
	Circle();
	Circle(float x_, float y_, float r_);
	~Circle();

	bool Collide(Circle other_);

	float x;
	float y;
	float r;
private:

};


