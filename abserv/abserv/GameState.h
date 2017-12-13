#pragma once

namespace Game {

class GameState
{
public:
    GameState() = default;
    ~GameState() = default;

    /// Comparison
    bool operator ==(const GameState& rhs) const { return Compare(rhs) == 0; }
    bool operator !=(const GameState& rhs) const { return Compare(rhs) != 0; }
    int Compare(const GameState& other) const;

    GameState& operator+=(const GameState& v);
    GameState& operator-=(const GameState& v);
    GameState operator+(const GameState& v) const;
    /// Get the difference of 2 game states
    GameState operator-(const GameState& v) const;

};

}
