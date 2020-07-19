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

void ItemStatsUIElement::SetStats(const String& value)
{
    if (value.Compare(stats_) == 0)
        return;
    stats_ = value;

    RemoveAllChildren();
    HashMap<Game::ItemStatIndex, Variant> stats;
    LoadStatsFromString(stats, stats_);
    AddDamageStats(stats);
    AddArmorStats(stats);

    SetVisible(GetNumChildren() != 0);
    SetStyleAuto();
    UpdateLayout();
}

void ItemStatsUIElement::AddDamageStats(const HashMap<Game::ItemStatIndex, Variant>& stats)
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

void ItemStatsUIElement::AddArmorStats(const HashMap<Game::ItemStatIndex, Variant>& stats)
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
