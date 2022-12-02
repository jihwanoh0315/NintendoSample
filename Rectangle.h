#pragma once
#include "Objects.h"
class Rectangle : public Objects
{
public:
	Rectangle();
	~Rectangle();
	Rectangle(float x1_,float y1_, float x2_, float y2_);

	void RenderInit();
	void Update();
	void SetPosition(float x_, float y_);
	void SetSize(float x_, float y_);
	bool inRect(float x_, float y_);
	void Render();

	float x1; //!< bottom left
	float y1;
	float x2; //!< top right
	float y2;
private:

};
