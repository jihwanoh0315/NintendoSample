#pragma once
#include "Objects.h"

class Circle : public Objects
{
public:
	Circle();
	Circle(float x_, float y_, float r_);
	~Circle();

	void RenderInit();
	bool Collide(Circle other_);
	bool inCircle(float x_, float y_);
	void Update();
	void Translate(float x_, float y_);
	void Render();

	float x;
	float y;
	float r;
private:

};


