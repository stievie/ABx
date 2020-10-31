/**
 * Copyright 2017-2020 Stefan Ascher
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

#include <abshared/TemplEncoder.h>
#include <Urho3DAll.h>

class Actor;

class SkillBarWindow : public Window
{
    URHO3D_OBJECT(SkillBarWindow, Window)
private:
    SharedArrayPtr<Button> skillButtons_;
    SharedPtr<Button> skill1_;
    SharedPtr<Button> skill2_;
    SharedPtr<Button> skill3_;
    SharedPtr<Button> skill4_;
    SharedPtr<Button> skill5_;
    SharedPtr<Button> skill6_;
    SharedPtr<Button> skill7_;
    SharedPtr<Button> skill8_;
    WeakPtr<Actor> actor_;
    Game::SkillIndices skills_;
    SharedPtr<Window> dragSkill_;
    void SubscribeEvents();
    void HandleUpdate(StringHash eventType, VariantMap& eventData);
    void HandleSkill1Clicked(StringHash eventType, VariantMap& eventData);
    void HandleSkill2Clicked(StringHash eventType, VariantMap& eventData);
    void HandleSkill3Clicked(StringHash eventType, VariantMap& eventData);
    void HandleSkill4Clicked(StringHash eventType, VariantMap& eventData);
    void HandleSkill5Clicked(StringHash eventType, VariantMap& eventData);
    void HandleSkill6Clicked(StringHash eventType, VariantMap& eventData);
    void HandleSkill7Clicked(StringHash eventType, VariantMap& eventData);
    void HandleSkill8Clicked(StringHash eventType, VariantMap& eventData);
    void HandleSetSkill(StringHash eventType, VariantMap& eventData);
    Button* GetButtonFromIndex(uint32_t index);
    void ResetSkillButtons();
    IntVector2 GetButtonSize() const;
    unsigned GetSkillPosFromClientPos(const IntVector2& clientPos);
    void UpdateSkill(unsigned pos, uint32_t index);
    bool IsChangeable() const;
    bool IsUseable() const;
    void ShowSkillsWindow();
    void HandleSkillDragBegin(StringHash eventType, VariantMap& eventData);
    void HandleSkillDragMove(StringHash eventType, VariantMap& eventData);
    void HandleSkillDragCancel(StringHash eventType, VariantMap& eventData);
    void HandleSkillDragEnd(StringHash eventType, VariantMap& eventData);
public:
    static void RegisterObject(Context* context);

    SkillBarWindow(Context* context);
    ~SkillBarWindow() override;

    void SetActor(SharedPtr<Actor> actor);
    void SetSkills(const Game::SkillIndices& skills);
    void DropSkill(const IntVector2& pos, uint32_t skillIndex);
};

