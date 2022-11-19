#pragma once
class Player
{
public:
	Player();
	Player(float x_, float y_);
	~Player();


	void Translate(float x_, float y_);

	// Positon of player
	float x;
	float y;
private:

};
