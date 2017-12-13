#include "stdafx.h"
#include "GameState.h"

namespace Game {

int GameState::Compare(const GameState& other) const
{
    return 0;
}

GameState& GameState::operator+=(const GameState& v)
{
    return *this;
}

GameState& GameState::operator-=(const GameState& v)
{
    return *this;
}

GameState GameState::operator+(const GameState& v) const
{
    return GameState();
}

GameState GameState::operator-(const GameState& v) const
{
    return GameState();
}

}
