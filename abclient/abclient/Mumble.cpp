#include "stdafx.h"
#include "Mumble.h"

#ifndef _WIN32
#include <unistd.h>
#include <sys/types.h>
#endif // _WIN32
// https://wiki.mumble.info/wiki/Link

Mumble::Mumble(Context* context) :
    Object(context),
#ifdef _WIN32
    hMapObject_(NULL),
#endif
    lm_(nullptr),
    avatar_(nullptr),
    camera_(nullptr),
    identityDirty_(false),
    contextDirty_(false),
    initialized_(false)
{
}

Mumble::~Mumble()
{
    Shutdown();
}

void Mumble::Initialize()
{
    if (initialized_)
        return;

#ifdef _WIN32
    hMapObject_ = OpenFileMappingW(FILE_MAP_ALL_ACCESS, FALSE, L"MumbleLink");
    if (hMapObject_ == NULL)
    {
        // Mumble is not running
        URHO3D_LOGWARNING("Mumble client not running");
        return;
    }

    lm_ = (LinkedMem *)MapViewOfFile(hMapObject_, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(LinkedMem));

    if (lm_ == NULL)
    {
        CloseHandle(hMapObject_);
        hMapObject_ = NULL;
        return;
    }
#else
    snprintf(memname_, 256, "/MumbleLink.%d", getuid());

    shmfd_ = shm_open(memname_, O_RDWR, S_IRUSR | S_IWUSR);
    if (shmfd_ < 0)
    {
        URHO3D_LOGWARNING("Mumble client not running");
        return;
    }

    lm_ = (LinkedMem *)(mmap(NULL, sizeof(struct LinkedMem), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd_, 0));
    if (lm_ == (void *)(-1))
    {
        URHO3D_LOGERROR("Mumble::Initialize: mmap() failed");
        lm_ = NULL;
        return;
    }
#endif
    initialized_ = true;
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(Mumble, HandleUpdate));
}

void Mumble::Shutdown()
{
    UnsubscribeFromEvent(E_UPDATE);

#ifdef _WIN32
    if (lm_)
    {
        UnmapViewOfFile(lm_);
    }
    if (hMapObject_)
    {
        CloseHandle(hMapObject_);
        hMapObject_ = NULL;
    }
#else
    if (lm_ != (void *)(-1))
    {
        munmap(lm_, sizeof(struct LinkedMem));
    }
    if (shmfd_ >= 0)
    {
        shm_unlink(memname_);
    }
#endif
    initialized_ = false;
}

void Mumble::HandleUpdate(StringHash, VariantMap&)
{
    if (!lm_ || !initialized_)
        return;

    if (lm_->uiVersion != 2)
    {
        wcsncpy(lm_->name, L"FW", 256);
        wcsncpy(lm_->description, L"FW Link plugin.", 2048);
        lm_->uiVersion = 2;
    }

    lm_->uiTick++;

    if (SharedPtr<Node> avatar = avatar_.Lock())
    {
        // Left handed coordinate system.
        // X positive towards "right".
        // Y positive towards "up".
        // Z positive towards "front".
        //
        // 1 unit = 1 meter

        const Quaternion& rot = avatar->GetWorldRotation();
        // Unit vector pointing out of the avatar's eyes aka "At"-vector.
        const Vector3 targetForward = rot * Vector3::FORWARD;
        lm_->fAvatarFront[0] = targetForward.x_;
        lm_->fAvatarFront[1] = targetForward.y_;
        lm_->fAvatarFront[2] = targetForward.z_;

        // Unit vector pointing out of the top of the avatar's head aka "Up"-vector (here Top points straight up).
        const Vector3 targetUp = rot * Vector3::UP;
        lm_->fAvatarTop[0] = targetUp.x_;
        lm_->fAvatarTop[1] = targetUp.y_;
        lm_->fAvatarTop[2] = targetUp.z_;

        // Position of the avatar (here standing slightly off the origin)
        const Vector3 worldPos = avatar->GetWorldPosition();
        lm_->fAvatarPosition[0] = worldPos.x_;
        lm_->fAvatarPosition[1] = worldPos.y_;
        lm_->fAvatarPosition[2] = worldPos.z_;
    }
    if (SharedPtr<Node> camera = camera_.Lock())
    {
        // Same as avatar but for the camera.

        const Vector3 worldPos = camera->GetWorldPosition();
        lm_->fCameraPosition[0] = worldPos.x_;
        lm_->fCameraPosition[1] = worldPos.y_;
        lm_->fCameraPosition[2] = worldPos.z_;

        const Quaternion& rot = camera->GetWorldRotation();
        const Vector3 targetForward = rot * Vector3::FORWARD;
        lm_->fCameraFront[0] = targetForward.x_;
        lm_->fCameraFront[1] = targetForward.y_;
        lm_->fCameraFront[2] = targetForward.z_;

        const Vector3 targetUp = rot * Vector3::UP;
        lm_->fCameraTop[0] = targetUp.x_;
        lm_->fCameraTop[1] = targetUp.y_;
        lm_->fCameraTop[2] = targetUp.z_;
    }

    if (identityDirty_)
    {
        // Identifier which uniquely identifies a certain player in a context (e.g. the ingame name).
        WString identity(identity_);
        wcsncpy(lm_->identity, identity.CString(), Min(256u, identity.Length()));
        identityDirty_ = false;
        URHO3D_LOGINFOF("Mumble identity is now %s", identity_.CString());
    }

    if (contextDirty_)
    {
        // Context should be equal for players which should be able to hear each other positional and
        // differ for those who shouldn't (e.g. it could contain the server+port and team)
        // E.g. Map instance ID
        unsigned length = Min(256u, mumbleContext_.Length());
        if (length > 0)
            memcpy(lm_->context, mumbleContext_.CString(), length);
        lm_->context_len = length;
        contextDirty_ = false;
        URHO3D_LOGINFOF("Mumble context is now %s", mumbleContext_.CString());
    }
}
