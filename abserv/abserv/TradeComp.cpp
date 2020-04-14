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

#include "stdafx.h"
#include "TradeComp.h"
#include "Player.h"
#include <abscommon/NetworkMessage.h>
#include <AB/Packets/ServerPackets.h>
#include <AB/Packets/Packet.h>

namespace Game {
namespace Components {

TradeComp::TradeComp(Player& owner) :
    owner_(owner)
{
    owner_.SubscribeEvent<void(void)>(EVENT_ON_STUCK, std::bind(&TradeComp::OnStuck, this));
    owner_.SubscribeEvent<void(AB::GameProtocol::CreatureState, AB::GameProtocol::CreatureState)>(EVENT_ON_STATECHANGE,
        std::bind(&TradeComp::OnStateChange, this, std::placeholders::_1, std::placeholders::_2));
}

void TradeComp::Update(uint32_t)
{
    if (auto target = target_.lock())
    {
        if (state_ == TradeState::MoveToTarget)
        {
            // We need to move to the target
            if (!CheckRange())
            {
                MoveToTarget(target);
                return;
            }
            else
            {
                owner_.autorunComp_->Reset();
                owner_.stateComp_.Reset();
                StartTrading();
            }
        }
    }
    else
    {
        target_.reset();
        state_ = TradeState::Idle;
    }
}

void TradeComp::Write(Net::NetworkMessage&)
{
}

void TradeComp::Reset()
{
    state_ = TradeState::Idle;
    target_.reset();
}

void TradeComp::Cancel()
{
    if (state_ > TradeState::Idle)
    {
        state_ = TradeState::Idle;
        if (auto target = target_.lock())
        {
            target->tradeComp_->Cancel();
        }
        target_.reset();
    }
}

TradeComp::TradeError TradeComp::TradeWith(std::shared_ptr<Player> target)
{
    // Target is in a queue for a match -> not possible to trade with.
    if (!target)
    {
        if (state_ == TradeState::Idle)
            return TradeError::TargetInvalid;
    }
    if (target && target->IsQueueing())
        return TradeError::TargetQueing;
    if (target->tradeComp_->IsTrading())
        return TradeError::TargetTrading;

    target_ = target;
    if (target)
    {
        if (!CheckRange())
        {
            state_ = TradeState::MoveToTarget;
            MoveToTarget(target);
        }
        else
            StartTrading();
    }
    else
        state_ = TradeState::Idle;
    return TradeError::None;
}

void TradeComp::TradeReqeust(std::shared_ptr<Player> source)
{
    target_ = source;
    state_ = TradeState::Trading;
}

void TradeComp::MoveToTarget(std::shared_ptr<Player> target)
{
    if (!owner_.autorunComp_->IsAutoRun())
    {
        if (owner_.autorunComp_->Follow(target, false, RANGE_PICK_UP))
        {
            owner_.stateComp_.SetState(AB::GameProtocol::CreatureState::Moving);
            owner_.autorunComp_->SetAutoRun(true);
        }
        else
            // No way to get to the target
            state_ = TradeState::Idle;
    }
}

bool TradeComp::CheckRange()
{
    auto target = target_.lock();
    if (!target)
        return false;

    return owner_.GetDistance(target.get()) < RANGE_PICK_UP;
}

void TradeComp::StartTrading()
{
    if (auto target = target_.lock())
    {
        if (!target->tradeComp_->IsTrading())
        {
            state_ = TradeState::Trading;
            target->tradeComp_->TradeReqeust(owner_.GetPtr<Player>());
            owner_.TriggerTradeDialog(target->id_);
        }
        else
        {
            auto msg = Net::NetworkMessage::GetNew();
            msg->AddByte(AB::GameProtocol::ServerPacketType::PlayerError);
            AB::Packets::Server::GameError packet{ AB::GameProtocol::PlayerErrorTradingPartnerTrading };
            AB::Packets::Add(packet, *msg);
            owner_.WriteToOutput(*msg);
            Reset();
        }
    }
    else
        Reset();
}

void TradeComp::OnStuck()
{
    if (state_ == TradeState::MoveToTarget)
    {
        owner_.autorunComp_->Reset();
        owner_.stateComp_.Reset();
        target_.reset();
        state_ = TradeState::Idle;
    }
}

void TradeComp::OnStateChange(AB::GameProtocol::CreatureState, AB::GameProtocol::CreatureState newState)
{
    if (state_ == TradeState::MoveToTarget)
    {
        if (newState != AB::GameProtocol::CreatureState::Moving)
            Reset();
    }
}

}
}
