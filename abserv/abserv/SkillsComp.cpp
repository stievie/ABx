#include "stdafx.h"
#include "SkillsComp.h"
#include "Actor.h"
#include "Skill.h"

namespace Game {
namespace Components {

void SkillsComp::Update(uint32_t timeElapsed)
{
    SkillBar* sb = owner_.GetSkillBar();
    sb->Update(timeElapsed);
    if (usingSkill_)
    {
        if (auto ls = lastSkill_.lock())
        {
            if (!ls->IsUsing())
            {
                usingSkill_ = false;
                endDirty_ = true;
                lastError_ = ls->GetLastError();
                newRecharge_ = ls->recharge_;
            }
        }
    }
}

AB::GameProtocol::SkillError SkillsComp::UseSkill(int index, bool ping)
{
    std::shared_ptr<Actor> target;
    if (auto selObj = owner_.selectedObject_.lock())
    {
        target = selObj->GetThisDynamic<Actor>();
    }
    // Can use skills only on Creatures not all GameObjects.
    // But a target is not mandatory, the Skill script will decide
    // if it needs a target, and may fail.
    SkillBar* sb = owner_.GetSkillBar();
    lastSkill_ = sb->GetSkill(index);
    lastError_ = sb->UseSkill(index, target);
    usingSkill_ = lastError_ == AB::GameProtocol::SkillErrorNone;
    lastSkillIndex_ = index;
    lastSkillTime_ = Utils::Tick();
    startDirty_ = true;
    endDirty_ = false;
    if (usingSkill_ && ping)
    {
        owner_.OnPingObject(target ? target->id_ : 0, AB::GameProtocol::ObjectCallTypeUseSkill, lastSkillIndex_ + 1);
    }
    return lastError_;
}

void SkillsComp::Cancel()
{
    if (usingSkill_)
    {
        if (auto ls = lastSkill_.lock())
        {
            if (ls->IsUsing())
            {
                usingSkill_ = false;
                endDirty_ = true;
                newRecharge_ = 0;
                ls->CancelUse();
            }
        }
    }
}

void SkillsComp::CancelWhenChangingState()
{
    if (usingSkill_)
    {
        if (auto ls = lastSkill_.lock())
        {
            if (ls->IsUsing() && ls->IsChangingState())
            {
                usingSkill_ = false;
                endDirty_ = true;
                newRecharge_ = 0;
                ls->CancelUse();
            }
        }
    }
}

bool SkillsComp::IsUsing()
{
    if (auto ls = lastSkill_.lock())
        return ls->IsUsing();
    return false;
}

void SkillsComp::Write(Net::NetworkMessage& message)
{
    // The server has 0 based skill indices but for the client these are 1 based
    if (startDirty_)
    {
        if (lastError_ == AB::GameProtocol::SkillErrorNone)
        {
            message.AddByte(AB::GameProtocol::GameObjectUseSkill);
            message.Add<uint32_t>(owner_.id_);
            message.Add<uint8_t>(static_cast<uint8_t>(lastSkillIndex_ + 1));
            if (auto ls = lastSkill_.lock())
            {
                message.Add<uint16_t>(static_cast<uint16_t>(ls->GetRealEnergy()));
                message.Add<uint16_t>(static_cast<uint16_t>(ls->GetRealAdrenaline()));
                message.Add<uint16_t>(static_cast<uint16_t>(ls->GetRealActivation()));
                message.Add<uint16_t>(static_cast<uint16_t>(ls->GetRealOvercast()));
                message.Add<uint16_t>(static_cast<uint16_t>(ls->GetRealHp()));
            }
            else
            {
                message.Add<uint16_t>(0);
                message.Add<uint16_t>(0);
                message.Add<uint16_t>(0);
                message.Add<uint16_t>(0);
                message.Add<uint16_t>(0);
            }
        }
        else
        {
            message.AddByte(AB::GameProtocol::GameObjectSkillFailure);
            message.Add<uint32_t>(owner_.id_);
            message.Add<int8_t>(static_cast<uint8_t>(lastSkillIndex_ + 1));
            message.AddByte(lastError_);
        }
        startDirty_ = false;
    }

    if (endDirty_)
    {
        if (lastError_ == AB::GameProtocol::SkillErrorNone)
        {
            message.AddByte(AB::GameProtocol::GameObjectEndUseSkill);
            message.Add<uint32_t>(owner_.id_);
            message.Add<int8_t>(static_cast<uint8_t>(lastSkillIndex_ + 1));
            message.Add<uint16_t>(static_cast<uint16_t>(newRecharge_));
        }
        else
        {
            message.AddByte(AB::GameProtocol::GameObjectSkillFailure);
            message.Add<uint32_t>(owner_.id_);
            message.Add<int8_t>(static_cast<uint8_t>(lastSkillIndex_ + 1));
            message.AddByte(lastError_);
        }
        endDirty_ = false;
    }
}

bool SkillsComp::Interrupt(AB::Entities::SkillType type)
{
    if (auto ls = lastSkill_.lock())
    {
        if (ls->IsUsing() && ls->IsType(type))
            return ls->Interrupt();
    }
    return false;
}

Skill* SkillsComp::GetCurrentSkill()
{
    if (auto ls = lastSkill_.lock())
    {
        if (ls->IsUsing())
        {
            return ls.get();
        }
    }
    return nullptr;
}

}
}
