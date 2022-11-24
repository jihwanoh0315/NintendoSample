#pragma once
#include "Objects.h"
class Rectangle : public Objects
{
public:
	Rectangle();
	~Rectangle();
	Rectangle(float x1_,float y1_, float x2_, float y2_);

	void Update();

	bool inRect(float x_, float y_);

	float x1; //!< bottom left
	float y1;
	float x2; //!< top right
	float y2;
private:

};
