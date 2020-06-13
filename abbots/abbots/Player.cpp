#include "stdafx.h"
#include "Player.h"

Player::Player(Type type, uint32_t id) :
    GameObject(type, id)
{

}

void Player::Update(uint32_t timeElapsed)
{
    GameObject::Update(timeElapsed);;
}
