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

#include "DialogWindow.h"
#include "AB/Entities/ConcreteItem.h"
#include <set>

class Actor;
class Player;

class TradeDialog : public DialogWindow
{
    URHO3D_OBJECT(TradeDialog, DialogWindow)
private:
    WeakPtr<Player> player_;
    WeakPtr<Actor> partner_;
    std::set<uint16_t> ourOffer_;
    std::set<uint32_t> partnerOffer_;
    void HandleOfferClicked(StringHash eventType, VariantMap& eventData);
    void HandleAcceptClicked(StringHash eventType, VariantMap& eventData);
    void HandleCancelClicked(StringHash eventType, VariantMap& eventData);
    void HandleMoneyEditTextEntry(StringHash eventType, VariantMap& eventData);
public:
    TradeDialog(Context* context, SharedPtr<Player> player, SharedPtr<Actor> partner);
    ~TradeDialog() override;
    bool DropItem(const IntVector2& screenPos, AB::Entities::StoragePlace currentPlace, uint16_t currItemPos);
};
