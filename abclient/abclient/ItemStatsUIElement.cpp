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

void ItemStatsUIElement::SetStats(const ItemStats& value)
{
    RemoveAllChildren();
    AddDamageStats(value);
    AddArmorStats(value);
    AddAttributeIncreaseStats(value);
    AddOtherStats(value);

    SetVisible(GetNumChildren() != 0);
    SetStyleAuto();
    UpdateLayout();
}

void ItemStatsUIElement::AddDamageStats(const ItemStats& stats)
{
    const auto minDmgIt = stats.Find(Game::ItemStatIndex::MinDamage);
    const auto maxDmgIt = stats.Find(Game::ItemStatIndex::MaxDamage);
    const auto attrIt = stats.Find(Game::ItemStatIndex::Attribute);
    if (minDmgIt == stats.End() && maxDmgIt == stats.End())
        return;

    const auto dmgType = stats.Find(Game::ItemStatIndex::DamageType);
    Text* damageText = CreateChild<Text>();
    damageText->SetInternal(true);
    String damageString;
    if (dmgType != stats.End())
    {
        const char* dmgTypeName = Game::GetDamageTypeName(static_cast<Game::DamageType>(dmgType->second_.GetUInt()));
        if (dmgTypeName != nullptr)
        {
            damageString.AppendWithFormat("%s-", dmgTypeName);
        }
    }
    damageString.Append("Damage: ");
    if (minDmgIt != stats.End())
        damageString.AppendWithFormat("%d", minDmgIt->second_.GetUInt());
    if (maxDmgIt != stats.End())
        damageString.AppendWithFormat("-%d", maxDmgIt->second_.GetUInt());

    if (attrIt != stats.End())
    {
        if (attrIt->second_.GetUInt() != static_cast<uint32_t>(Game::ItemStatIndex::None))
        {
            auto* sm = GetSubsystem<SkillManager>();
            const auto* attrib = sm->GetAttributeByIndex(attrIt->second_.GetUInt());
            if (attrib)
            {
                damageString.Append(" (Requires");
                const auto itValue = stats.Find(Game::ItemStatIndex::AttributeValue);
                if (itValue != stats.End())
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

void ItemStatsUIElement::AddArmorStats(const ItemStats& stats)
{
    const auto armorIt = stats.Find(Game::ItemStatIndex::Armor);
    const auto attrIt = stats.Find(Game::ItemStatIndex::Attribute);
    if (armorIt == stats.End())
        return;

    Text* armorText = CreateChild<Text>();
    armorText->SetInternal(true);
    String armorString;
    armorString.AppendWithFormat("Armor: %d", armorIt->second_.GetUInt());

    if (attrIt != stats.End())
    {
        if (attrIt->second_.GetUInt() != static_cast<uint32_t>(Game::ItemStatIndex::None))
        {
            auto* sm = GetSubsystem<SkillManager>();
            const auto* attrib = sm->GetAttributeByIndex(attrIt->second_.GetUInt());
            if (attrib)
            {
                armorString.Append(" (Requires");
                const auto itValue = stats.Find(Game::ItemStatIndex::AttributeValue);
                if (itValue != stats.End())
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

void ItemStatsUIElement::AddAttributeIncreaseStats(const ItemStats& stats)
{
    auto addStat = [this](const char* name, uint32_t value)
    {
        Text* text = CreateChild<Text>();
        String string;
        string.AppendWithFormat("+%u %s", value, name);
        text->SetInternal(true);
        text->SetText(string);
    };
    auto* sm = GetSubsystem<SkillManager>();
    for (size_t i = static_cast<size_t>(Game::ItemStatIndex::AttributeOffset); i <= static_cast<size_t>(Game::ItemStatIndex::__AttributeLast); ++i)
    {
        const auto it = stats.Find(static_cast<Game::ItemStatIndex>(i));
        if (it == stats.End())
            continue;
        if (it->second_ == 0)
            continue;
        const auto* attrib = sm->GetAttributeByIndex(static_cast<uint32_t>(i - static_cast<size_t>(Game::ItemStatIndex::AttributeOffset)));
        if (!attrib)
            continue;
        addStat(attrib->name.c_str(), it->second_.GetUInt());
    }
}

void ItemStatsUIElement::AddOtherStats(const ItemStats& stats)
{
    auto addStat = [this](const String& string)
    {
        Text* text = CreateChild<Text>();
        text->SetInternal(true);
        text->SetText(string);
    };
    auto addStatSign = [this](const String& format, int value)
    {
        Text* text = CreateChild<Text>();
        char sign = value > 0 ? '+' : '-';
        String string = String(sign);
        string.AppendWithFormat(format.CString(), value);
        text->SetInternal(true);
        text->SetText(string);
    };
    for (const auto& stat : stats)
    {
        switch (stat.first_)
        {
        case Game::ItemStatIndex::HealthRegen:
        {
            addStatSign("%d Health regeneration", stat.second_.GetInt());
            break;
        }
        case Game::ItemStatIndex::EnergyRegen:
        {
            addStatSign("%d Energy regeneration", stat.second_.GetInt());
            break;
        }
        case Game::ItemStatIndex::Health:
        {
            addStatSign("%d Health", stat.second_.GetInt());
            break;
        }
        case Game::ItemStatIndex::Energy:
        {
            addStatSign("%d Energy", stat.second_.GetInt());
            break;
        }
        case Game::ItemStatIndex::PhysicalDamageReduction:
        {
            addStatSign("%d Damage", stat.second_.GetInt());
            break;
        }
        case Game::ItemStatIndex::HexDurationReduction:
        {
            addStatSign("%d Hex duration", stat.second_.GetInt());
            break;
        }
        case Game::ItemStatIndex::ConditionDurationReduction:
        {
            addStatSign("%d Condition duration", stat.second_.GetInt());
            break;
        }
        case Game::ItemStatIndex::BlindnessDurationReduction:
        {
            addStatSign("%d Blindness duration", stat.second_.GetInt());
            break;
        }
        case Game::ItemStatIndex::WeaknessDurationReduction:
        {
            addStatSign("%d Weakness duration", stat.second_.GetInt());
            break;
        }
        case Game::ItemStatIndex::DeseaseDurationReduction:
        {
            addStatSign("%d Desease duration", stat.second_.GetInt());
            break;
        }
        case Game::ItemStatIndex::PoisionDurationReduction:
        {
            addStatSign("%d Poison duration", stat.second_.GetInt());
            break;
        }
        case Game::ItemStatIndex::DazedDurationReduction:
        {
            addStatSign("%d Dazed duration", stat.second_.GetInt());
            break;
        }
        case Game::ItemStatIndex::DeepWoundDurationReduction:
        {
            addStatSign("%d Deep wound duration", stat.second_.GetInt());
            break;
        }
        case Game::ItemStatIndex::BleedingDurationReduction:
        {
            addStatSign("%d Bleeding duration", stat.second_.GetInt());
            break;
        }
        case Game::ItemStatIndex::CrippledDurationReduction:
        {
            addStatSign("%d Crippled duration", stat.second_.GetInt());
            break;
        }
        case Game::ItemStatIndex::ArmorElemental:
        {
            addStatSign("%d armor vs. elemental damage", stat.second_.GetInt());
            break;
        }
        case Game::ItemStatIndex::ArmorFire:
        {
            addStatSign("%d armor vs. fire damage", stat.second_.GetInt());
            break;
        }
        case Game::ItemStatIndex::ArmorCold:
        {
            addStatSign("%d armor vs. cold damage", stat.second_.GetInt());
            break;
        }
        case Game::ItemStatIndex::ArmorLightning:
        {
            addStatSign("%d armor vs. lightning damage", stat.second_.GetInt());
            break;
        }
        case Game::ItemStatIndex::ArmorEarth:
        {
            addStatSign("%d armor vs. earth damage", stat.second_.GetInt());
            break;
        }
        case Game::ItemStatIndex::ArmorPhysical:
        {
            addStatSign("%d armor vs. physical damage", stat.second_.GetInt());
            break;
        }
        case Game::ItemStatIndex::ArmorDark:
        case Game::ItemStatIndex::ArmorTypeless:
        case Game::ItemStatIndex::ArmorShadow:
        case Game::ItemStatIndex::ArmorHoly:
        case Game::ItemStatIndex::ArmorChaos:
            // Armor ignoring damage
            ASSERT_FALSE();
        case Game::ItemStatIndex::Usages:
        {
            String string;
            int value = stat.second_.GetInt();
            const char* text = (value == 1) ? "%d Usage" : "%d Usages";
            string.AppendWithFormat(text, value);
            addStat(string);
            break;
        }
        default:
            break;
        }
    }
}
