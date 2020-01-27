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

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/mman.h>
#include <fcntl.h> /* For O_* constants */
#endif // _WIN32
#include <stdint.h>
#include <Urho3DAll.h>

struct LinkedMem
{
    uint32_t uiVersion;
    uint32_t uiTick;
    float	fAvatarPosition[3];
    float	fAvatarFront[3];
    float	fAvatarTop[3];
    wchar_t	name[256];
    float	fCameraPosition[3];
    float	fCameraFront[3];
    float	fCameraTop[3];
    wchar_t	identity[256];
    uint32_t context_len;
    unsigned char context[256];
    wchar_t description[2048];
};

class Mumble : public Object
{
    URHO3D_OBJECT(Mumble, Object)
private:
#ifdef _WIN32
    HANDLE hMapObject_;
#else
    int shmfd_;
    char memname_[256];
#endif
    LinkedMem* lm_;
    WeakPtr<Node> avatar_;
    WeakPtr<Node> camera_;
    String identity_;
    bool identityDirty_;
    String mumbleContext_;
    bool contextDirty_;
    bool initialized_;
    void HandleUpdate(StringHash eventType, VariantMap& eventData);
public:
    Mumble(Context* context);
    ~Mumble() override;
    /// Connect to Mumble
    void Initialize();
    /// Disconnect from Mumble
    void Shutdown();
    /// Character Node
    void SetAvatar(SharedPtr<Node> avatar)
    {
        avatar_ = avatar;
    }
    /// Camera Node
    void SetCamera(SharedPtr<Node> camera)
    {
        camera_ = camera;
    }
    /// Your identity, e.g. ingame name
    void SetIdentity(const String& identity)
    {
        if (identity_.Compare(identity) != 0)
        {
            identity_ = identity;
            identityDirty_ = true;
        }
    }
    void SetContext(const String& context)
    {
        if (mumbleContext_.Compare(context) != 0)
        {
            mumbleContext_ = context;
            contextDirty_ = true;
        }
    }
    bool IsInitialized() const { return initialized_; }
};

