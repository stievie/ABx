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
#include <AB/Entities/Game.h>

class Player;
class GameObject;

class MissionMapWindow : public Window
{
    URHO3D_OBJECT(MissionMapWindow, Window)
public:
    static void RegisterObject(Context* context);

    MissionMapWindow(Context* context);
    ~MissionMapWindow() override;
    void SetScene(SharedPtr<Scene> scene, AB::Entities::GameType gameType, const String& sceneFile);
private:
    enum class DotType
    {
        Self,
        Ally,
        Foe,
        Other,
        Waypoint,
        PingPos,
        Marker,
    };
    enum class PingType
    {
        None,
        Position,
        Target
    };
    static const Color SELF_COLOR;
    static const Color ALLY_COLOR;
    static const Color FOE_COLOR;
    static const Color OTHER_COLOR;
    static const Color WAYPOINT_COLOR;
    static const Color PING_COLOR;
    static const Color MARKER_COLOR;
    int64_t pingTime_{ 0 };
    uint32_t pingerId_{ 0 };
    Vector3 pingPos_;
    PingType pingType_{ PingType::None };
    WeakPtr<GameObject> target_;

    SharedPtr<Texture2D> mapTexture_;
    SharedPtr<Image> mapImage_;
    SharedPtr<Texture2D> heightmapTexture_;
    SharedPtr<BorderImage> terrainLayer_;
    SharedPtr<BorderImage> objectLayer_;
    Vector<Vector3> waypoints_;
    Vector3 marker_;
    bool haveMarker_{ false };
    Vector3 terrainSpacing_;
    Vector3 terrainWorldSize_;
    Vector2 terrainScaling_;
    AB::Entities::GameType gameType_{ AB::Entities::GameTypeUnknown };
    void FitTexture();
    Player* GetPlayer() const;
    void DrawObjects();
    void SubscribeToEvents();
    void HandleCloseClicked(StringHash eventType, VariantMap& eventData);
    void HandleUpdate(StringHash eventType, VariantMap& eventData);
    void HandleRenderUpdate(StringHash eventType, VariantMap& eventData);
    void HandleResized(StringHash eventType, VariantMap& eventData);
    void HandlePositionPinged(StringHash eventType, VariantMap& eventData);
    void HandleMouseDown(StringHash eventType, VariantMap& eventData);
    void HandleTargetPinged(StringHash eventType, VariantMap& eventData);
    IntVector2 WorldToMapPos(const Vector3& center, const Vector3& world) const;
    IntVector2 WorldToMap(const Vector3& world) const;
    Vector3 MapToWorldPos(const Vector3& center, const IntVector2& map) const;
    Vector3 MapToWorld(const IntVector2& map) const;
    void DrawObject(const IntVector2& pos, DotType type);
};

