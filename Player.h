#pragma once
#include "Objects.h"

class Player : public Objects
{
public:
	Player();
	Player(float x_, float y_);
	~Player();

	void Update();

	void Translate(float x_, float y_);

	// Positon of player
	float x;
	float y;
private:

};
