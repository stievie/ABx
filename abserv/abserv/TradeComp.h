/**
 * Copyright 2020 Stefan Ascher
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once

#include <sa/Noncopyable.h>
#include <memory>
#include <AB/ProtocolCodes.h>

namespace Net {
class NetworkMessage;
}

namespace Game {

class Player;

namespace Components {

class TradeComp
{
    NON_COPYABLE(TradeComp)
    NON_MOVEABLE(TradeComp)
private:
    enum class TradeState
    {
        Idle,
        MoveToTarget,
        Trading,
    };
    Player& owner_;
    std::weak_ptr<Player> target_;
    TradeState state_{ TradeState::Idle };
    void MoveToTarget(std::shared_ptr<Player> target);
    bool CheckRange();
    void StartTrading();
    void OnStuck();
    void OnStateChange(AB::GameProtocol::CreatureState oldState, AB::GameProtocol::CreatureState newState);
public:
    enum class TradeError
    {
        None,
        TargetInvalid,
        TargetTrading,
        TargetQueing,
    };
    TradeComp() = delete;
    explicit TradeComp(Player& owner);
    ~TradeComp() = default;
    void Update(uint32_t timeElapsed);
    void Write(Net::NetworkMessage& message);

    TradeError TradeWith(std::shared_ptr<Player> target);
    void TradeReqeust(std::shared_ptr<Player> source);
    void Reset();
    // One player requested to cancel the trading
    void Cancel();

    bool IsTrading() const { return state_ > TradeState::Idle; }
};

}
}
