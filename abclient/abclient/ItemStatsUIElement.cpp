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

#include "ItemStatsUIElement.h"
#include "FwClient.h"
#include "SkillManager.h"
#include <abshared/Damage.h>

void ItemStatsUIElement::RegisterObject(Context* context)
{
    context->RegisterFactory<ItemStatsUIElement>();
}

ItemStatsUIElement::ItemStatsUIElement(Context* context) :
    UIElement(context)
{
    SetDefaultStyle(GetSubsystem<UI>()->GetRoot()->GetDefaultStyle());
    SetLayoutMode(LM_VERTICAL);
    SetLayoutSpacing(2);
}

ItemStatsUIElement::~ItemStatsUIElement() = default;

void ItemStatsUIElement::SetStats(const HashMap<Game::ItemStatIndex, Variant>& value)
{
    stats_ = value;

    RemoveAllChildren();
    HashMap<Game::ItemStatIndex, Variant> stats;
    AddDamageStats();
    AddArmorStats();
    AddOtherStats();

    SetVisible(GetNumChildren() != 0);
    SetStyleAuto();
    UpdateLayout();
}

void ItemStatsUIElement::AddDamageStats()
{
    const auto minDmgIt = stats_.Find(Game::ItemStatIndex::MinDamage);
    const auto maxDmgIt = stats_.Find(Game::ItemStatIndex::MaxDamage);
    const auto attrIt = stats_.Find(Game::ItemStatIndex::Attribute);
    if (minDmgIt == stats_.End() && maxDmgIt == stats_.End())
        return;

    const auto dmgType = stats_.Find(Game::ItemStatIndex::DamageType);
    Text* damageText = CreateChild<Text>();
    damageText->SetInternal(true);
    String damageString;
    if (dmgType != stats_.End())
    {
        const char* dmgTypeName = Game::GetDamageTypeName(static_cast<Game::DamageType>(dmgType->second_.GetUInt()));
        if (dmgTypeName != nullptr)
        {
            damageString.AppendWithFormat("%s-", dmgTypeName);
        }
    }
    damageString.Append("Damage: ");
    if (minDmgIt != stats_.End())
        damageString.AppendWithFormat("%d", minDmgIt->second_.GetUInt());
    if (maxDmgIt != stats_.End())
        damageString.AppendWithFormat("-%d", maxDmgIt->second_.GetUInt());

    if (attrIt != stats_.End())
    {
        if (attrIt->second_.GetUInt() != static_cast<uint32_t>(Game::ItemStatIndex::None))
        {
            auto* sm = GetSubsystem<SkillManager>();
            const auto* attrib = sm->GetAttributeByIndex(attrIt->second_.GetUInt());
            if (attrib)
            {
                damageString.Append(" (Requires");
                const auto itValue = stats_.Find(Game::ItemStatIndex::AttributeValue);
                if (itValue != stats_.End())
                {
                    damageString.AppendWithFormat(" %d", itValue->second_.GetInt());
                }
                damageString.AppendWithFormat(" %s", attrib->name.c_str());
                damageString.Append(")");
            }
        }
    }
    damageText->SetText(damageString);
}

void ItemStatsUIElement::AddArmorStats()
{
    const auto armorIt = stats_.Find(Game::ItemStatIndex::Armor);
    const auto attrIt = stats_.Find(Game::ItemStatIndex::Attribute);
    if (armorIt == stats_.End())
        return;

    Text* armorText = CreateChild<Text>();
    armorText->SetInternal(true);
    String armorString;
    armorString.AppendWithFormat("Armor: %d", armorIt->second_.GetUInt());

    if (attrIt != stats_.End())
    {
        if (attrIt->second_.GetUInt() != static_cast<uint32_t>(Game::ItemStatIndex::None))
        {
            auto* sm = GetSubsystem<SkillManager>();
            const auto* attrib = sm->GetAttributeByIndex(attrIt->second_.GetUInt());
            if (attrib)
            {
                armorString.Append(" (Requires");
                const auto itValue = stats_.Find(Game::ItemStatIndex::AttributeValue);
                if (itValue != stats_.End())
                {
                    armorString.AppendWithFormat(" %d", itValue->second_.GetInt());
                }
                armorString.AppendWithFormat(" %s", attrib->name.c_str());
                armorString.Append(")");
            }
        }
    }
    armorText->SetText(armorString);
}

void ItemStatsUIElement::AddOtherStats()
{
    auto addStat = [this](const String& string)
    {
        Text* text = CreateChild<Text>();
        text->SetInternal(true);
        text->SetText(string);
    };
    for (const auto& stat : stats_)
    {
        switch (stat.first_)
        {
        case Game::ItemStatIndex::HealthRegen:
        {
            String string;
            string.AppendWithFormat("%d Health regeneration", stat.second_.GetInt());
            addStat(string);
            break;
        }
        case Game::ItemStatIndex::EnergyRegen:
        {
            String string;
            string.AppendWithFormat("%d Energy regeneration", stat.second_.GetInt());
            addStat(string);
            break;
        }
        case Game::ItemStatIndex::Health:
        {
            String string;
            string.AppendWithFormat("%d Health", stat.second_.GetInt());
            addStat(string);
            break;
        }
        case Game::ItemStatIndex::Energy:
        {
            String string;
            string.AppendWithFormat("%d Energy", stat.second_.GetInt());
            addStat(string);
            break;
        }
        case Game::ItemStatIndex::PhysicalDamageReduction:
        {
            String string;
            string.AppendWithFormat("%d Damage", stat.second_.GetInt());
            addStat(string);
            break;
        }
        case Game::ItemStatIndex::HexDurationReduction:
        {
            String string;
            string.AppendWithFormat("%d Hex duration", stat.second_.GetInt());
            addStat(string);
            break;
        }
        case Game::ItemStatIndex::ConditionDurationReduction:
        {
            String string;
            string.AppendWithFormat("%d Condition duration", stat.second_.GetInt());
            addStat(string);
            break;
        }
        case Game::ItemStatIndex::BlindnessDurationReduction:
        {
            String string;
            string.AppendWithFormat("%d Blindness duration", stat.second_.GetInt());
            addStat(string);
            break;
        }
        case Game::ItemStatIndex::WeaknessDurationReduction:
        {
            String string;
            string.AppendWithFormat("%d Weakness duration", stat.second_.GetInt());
            addStat(string);
            break;
        }
        case Game::ItemStatIndex::DeseaseDurationReduction:
        {
            String string;
            string.AppendWithFormat("%d Desease duration", stat.second_.GetInt());
            addStat(string);
            break;
        }
        case Game::ItemStatIndex::PoisionDurationReduction:
        {
            String string;
            string.AppendWithFormat("%d Poison duration", stat.second_.GetInt());
            addStat(string);
            break;
        }
        case Game::ItemStatIndex::DazedDurationReduction:
        {
            String string;
            string.AppendWithFormat("%d Dazed duration", stat.second_.GetInt());
            addStat(string);
            break;
        }
        case Game::ItemStatIndex::DeepWoundDurationReduction:
        {
            String string;
            string.AppendWithFormat("%d Deep wound duration", stat.second_.GetInt());
            addStat(string);
            break;
        }
        case Game::ItemStatIndex::BleedingDurationReduction:
        {
            String string;
            string.AppendWithFormat("%d Bleeding duration", stat.second_.GetInt());
            addStat(string);
            break;
        }
        case Game::ItemStatIndex::CrippledDurationReduction:
        {
            String string;
            string.AppendWithFormat("%d Crippled duration", stat.second_.GetInt());
            addStat(string);
            break;
        }
        case Game::ItemStatIndex::ArmorElemental:
        {
            String string;
            string.AppendWithFormat("%d Elemental armor", stat.second_.GetInt());
            addStat(string);
            break;
        }
        case Game::ItemStatIndex::ArmorFire:
        {
            String string;
            string.AppendWithFormat("%d Fire armor", stat.second_.GetInt());
            addStat(string);
            break;
        }
        case Game::ItemStatIndex::ArmorCold:
        {
            String string;
            string.AppendWithFormat("%d Cold armor", stat.second_.GetInt());
            addStat(string);
            break;
        }
        case Game::ItemStatIndex::ArmorLightning:
        {
            String string;
            string.AppendWithFormat("%d Lightning armor", stat.second_.GetInt());
            addStat(string);
            break;
        }
        case Game::ItemStatIndex::ArmorEarth:
        {
            String string;
            string.AppendWithFormat("%d Earth armor", stat.second_.GetInt());
            addStat(string);
            break;
        }
        case Game::ItemStatIndex::ArmorPhysical:
        {
            String string;
            string.AppendWithFormat("%d Physical armor", stat.second_.GetInt());
            addStat(string);
            break;
        }
        case Game::ItemStatIndex::ArmorHoly:
        {
            String string;
            string.AppendWithFormat("%d Holy armor", stat.second_.GetInt());
            addStat(string);
            break;
        }
        case Game::ItemStatIndex::ArmorShadow:
        {
            String string;
            string.AppendWithFormat("%d Shadow armor", stat.second_.GetInt());
            addStat(string);
            break;
        }
        case Game::ItemStatIndex::ArmorTypeless:
        {
            String string;
            string.AppendWithFormat("%d Typeless armor", stat.second_.GetInt());
            addStat(string);
            break;
        }
        case Game::ItemStatIndex::ArmorDark:
        {
            String string;
            string.AppendWithFormat("%d Dark armor", stat.second_.GetInt());
            addStat(string);
            break;
        }
        case Game::ItemStatIndex::ArmorChaos:
        {
            String string;
            string.AppendWithFormat("%d Chaos armor", stat.second_.GetInt());
            addStat(string);
            break;
        }
        case Game::ItemStatIndex::Usages:
        {
            String string;
            string.AppendWithFormat("%d Usages", stat.second_.GetInt());
            addStat(string);
            break;
        }
        default:
            break;
        }
    }
}
