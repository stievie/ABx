#pragma once

#pragma warning( push )
#pragma warning( disable : 4100)
#include <Urho3D/Urho3DAll.h>
#pragma warning( pop )
#include <stdint.h>

class Options : public Object
{
    URHO3D_OBJECT(Options, Object);
public:
    Options(Context* context);
    ~Options();

    String loginHost_;
    uint16_t loginPort_;

    void Load();

    int GetWidth() { return width_; }
    void SetWidth(int value);
    int GetHeight() { return height_; }
    void SetHeight(int value);
    bool GetFullscreen() { return fullscreen_; }
    void SetFullscreen(bool value);
    bool GetBorderless() { return borderless_; }
    void SetBorderless(bool value);
    bool GetResizeable() { return resizeable_; }
    void SetResizeable(bool value);
    bool GetVSync() { return vSync_; }
    void SetVSync(bool value);
    bool GetTripleBuffer() { return tripleBuffer_; }
    void SetTripleBuffer(bool value);
    bool GetHighDPI() { return highDPI_; }
    void SetHighDPI(bool value);
    void SetMultiSample(int value);
    int GetMultiSample() { return multiSample_; }
private:
    int width_;
    int height_;
    bool fullscreen_;
    bool borderless_;
    bool resizeable_;
    bool vSync_;
    bool tripleBuffer_;
    bool highDPI_;
    int multiSample_;

    void UpdateGraphicsMode();
};

