#include <nn/hid.h>
#include <nn/hid/hid_Npad.h>
#include <nn/hid/hid_NpadJoy.h>

#include "Player.h"

Player::Player()
{
}

Player::~Player()
{
}

void Player::Translate(float x_, float y_)
{
	x += x_;
	y += y_;
}
