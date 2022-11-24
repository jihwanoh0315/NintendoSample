#include <nn/hid.h>
#include <nn/hid/hid_Npad.h>
#include <nn/hid/hid_NpadJoy.h>

#include "Player.h"

Player::Player() : x(0.0f), y(0.0f)
{
}

Player::Player(float x_, float y_) : x(x_), y(y_)
{
}

Player::~Player()
{
}

void Player::Update()
{
}

void Player::Translate(float x_, float y_)
{
	x += x_;
	y += y_;
}
