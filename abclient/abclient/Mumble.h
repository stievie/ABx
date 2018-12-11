#pragma once

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/mman.h>
#include <fcntl.h> /* For O_* constants */
#endif // _WIN32

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
    URHO3D_OBJECT(Mumble, Object);
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
    String context_;
    bool contextDirty_;
    bool initialized_;
    void HandleUpdate(StringHash eventType, VariantMap& eventData);
public:
    Mumble(Context* context);
    ~Mumble();
    void Initialize();
    void Shutdown();
    void SetAvatar(SharedPtr<Node> avatar)
    {
        avatar_ = avatar;
    }
    void SetCamera(SharedPtr<Node> camera)
    {
        camera_ = camera;
    }
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
        if (context_.Compare(context) != 0)
        {
            context_ = context;
            contextDirty_ = true;
        }
    }
    bool IsInitialized() const { return initialized_; }
};

