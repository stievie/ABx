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

#include <Urho3DAll.h>

class Player;

class MissionMapWindow : public Window
{
    URHO3D_OBJECT(MissionMapWindow, Window)
public:
    static const IntRect DOT_NONE;
    static const IntRect DOT_GREEN;
    static const IntRect DOT_ORANGE;
    static const IntRect DOT_RED;
    static void RegisterObject(Context* context);

    MissionMapWindow(Context* context);
    ~MissionMapWindow() override;
    void OnDragBegin(const IntVector2& position,
        const IntVector2& screenPosition,
        MouseButtonFlags buttons, QualifierFlags qualifierFlags, Cursor* cursor) override;
    void SetScene(SharedPtr<Scene> scene);
private:
    SharedPtr<Texture2D> mapTexture_;
    SharedPtr<Image> mapImage_;
    SharedPtr<Texture2D> dots_;
    SharedPtr<Image> greenDot_;
    int zoom_;
    void FitTexture();
    Player* GetPlayer() const;
    void DrawObjects();
    void SubscribeToEvents();
    void HandleCloseClicked(StringHash eventType, VariantMap& eventData);
    void HandleUpdate(StringHash eventType, VariantMap& eventData);
    void HandleRenderUpdate(StringHash eventType, VariantMap& eventData);
    void HandlePostRenderUpdate(StringHash eventType, VariantMap& eventData);
    void HandleResized(StringHash eventType, VariantMap& eventData);
    IntVector2 WorldToMapPos(const Vector3& center, const Vector3& world) const;
    void DrawObject(const IntVector2& pos);
};

