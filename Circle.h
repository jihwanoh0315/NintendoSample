#pragma once
#include "Objects.h"

class Circle : public Objects
{
public:
	Circle();
	Circle(float x_, float y_, float r_);
	~Circle();

	bool Collide(Circle other_);
	bool inCircle(float x_, float y_);
	void Update();

	float x;
	float y;
	float r;
private:

};


