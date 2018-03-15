#pragma once

#include <Urho3D/Scene/Component.h>
#include <Urho3D/Container/Pair.h>

enum TonemapMode
{
    REINHARDEQ3,
    REINHARDEQ4,
    UNCHARTED2,
    TONEMAP_DISABLED
};

namespace Urho3D
{
    class Renderer;
    class Viewport;
    class RenderPath;
    struct ResourceRef;
    class Texture3D;
}
//typedef Pair<const String&, const String&> TagEffectPair;

using namespace Urho3D;
using TagEffectPair = Pair< String,  String>;

class PostProcessController : public Component
{
    URHO3D_OBJECT(PostProcessController, Component);

public:
    PostProcessController(Context* context);
    ~PostProcessController();

    static void RegisterObject(Context* context);

    bool IsUsingAutoExposure() const { return useAutoExposure_; }
    void SetUseAutoExposure(bool val);
    unsigned GetAutoExposureOrder() const;
    void SetAutoExposureOrder(unsigned val);
    float GetAutoExposureAdaptRate() const { return aeAdaptRate_; }
    void SetAutoExposureAdaptRate(float val);
    const Vector2& GetAutoExposureLumRange() const { return aeLumRange_; }
    void SetAutoExposureLumRange(const Vector2& val);
    float GetAutoExposureMidGrey() const { return aeMiddleGrey_; }
    void SetAutoExposureMidGrey(float val);
    bool IsUsingBloom() const { return useBloom_; }
    void SetUseBloom(bool val);
    unsigned GetBloomOrder() const;
    void SetBloomOrder(unsigned val);
    float GetBloomThreshold() const { return bloomThreshold_; }
    void SetBloomThreshold(float val);
    Vector2 GetBloomMix() const { return bloomMix_; }
    void SetBloomMix(const Vector2& val);
    bool IsUsingBloomHDR() const { return useBloomHDR_; }
    void SetUseBloomHDR(bool val);
    unsigned GetBloomHDROrder() const;
    void SetBloomHDROrder(unsigned value);
    float GetBloomHDRThreshold() const { return bloomHDRThreshold_; }
    void SetBloomHDRThreshold(float val);
    Vector2 GetBloomHDRMix() const { return bloomHDRMix_; }
    void SetBloomHDRMix(const Vector2& val);
    Vector2 GetBloomHDRDirH() const { return bloomHDRDirH_; }
    void SetBloomHDRDirH(const Vector2& val);
    Vector2 GetBloomHDRDirV() const { return bloomHDRDirV_; }
    void SetBloomHDRDirV(const Vector2& val);
    float GetBloomHDRBlurRadius() const { return bloomHDRBlurRadius_; }
    void SetBloomHDRBlurRadius(float val);
    float GetBloomHDRBlurSigma() const { return bloomHDRBlurSigma_; }
    void SetBloomHDRBlurSigma(float val);
    bool IsUsingBlur() const { return useBlur_; }
    void SetUseBlur(bool val);
    unsigned GetBlurOrder() const;
    void SetBlurOrder(unsigned value);
    Vector2 GetBlurDirH() const { return blurDirH_; }
    void SetBlurDirH(const Vector2& val);
    Vector2 GetBlurDirV() const { return blurDirV_; }
    void SetBlurDirV(const Vector2& val);
    float GetBlurRadius() const { return blurRadius_; }
    void SetBlurRadius(float val);
    float GetBlurSigma() const { return blurSigma_; }
    void SetBlurSigma(float val);
    bool IsUsingFXAA2() const { return useFXAA2_; }
    void SetUseFXAA2(bool val);
    unsigned GetFXAA2Order() const;
    void SetFXAA2Order(unsigned val);
    Vector3 GetFXAAParams() const { return FXAAParams_; }
    void SetFXAAParams(const Vector3& val);
    bool IsUsingFXAA3() const { return useFXAA3_; }
    void SetUseFXAA3(bool val);
    unsigned GetFXAA3Order() const;
    void SetFXAA3Order(unsigned val);
    unsigned GetFXAA3QualityPreset() const { return FXAA3QualityPreset_; }
    void SetFXAA3QualityPreset(unsigned val);
    bool IsUsingGammaCorrection() const { return useGammaCorrection_; }
    void SetUseGammaCorrection(bool val);
    unsigned GetGammaCorrectionOrder() const;
    void SetGammaCorrectionOrder(unsigned val);
    TonemapMode GetTonemapMode() const { return tonemapMode_; }
    void SetTonemapMode(TonemapMode val);
    unsigned GetTonemapOrder() const;
    void SetToneMapOrder(unsigned val);
    float GetTonemapExposureBias() const { return tonemapExposureBias_; }
    void SetTonemapExposureBias(float val);
    float GetTonemapMaxWhite() const { return tonemapMaxWhite_; }
    void SetTonemapMaxWhite(float val);
    bool IsUsingColorCorrection() const { return useColorCorrection_; }
    void SetUseColorCorrection(bool val);
    unsigned GetColorCorrectionOrder() const;
    void SetColorCorrectionOrder(unsigned val);
    bool IsUsingGreyScale() const { return useGreyScale_; }
    void SetUseGreyScale(bool val);
    unsigned GetGreyScaleOrder() const;
    void SetGreyScaleOrder(unsigned val);

    Texture3D* GetLutTexture();
    void SetLutTexture(Texture3D* val);
    ResourceRef GetLutTextureAttr() const;
    void SetLutTextureAttr(const ResourceRef& val);

    const Vector <WeakPtr<Viewport>>& GetViewports() const { return viewports_; }
    unsigned GetNumViewports() const { return viewports_.Size(); }
    void ClearViewports();
    bool RemoveViewport(Viewport* target);
    void AddViewport(Viewport* target, bool clearList = false);
    bool RemoveViewportIndex(unsigned index, bool removeViewport = true);
    void AddViewportIndex(unsigned index, bool clearList = false);
    RenderPath* BuildEffectsPath();
    RenderPath* GetEffectsPath();
    ResourceRef GetInputPathAttr() const { return inputPathAttr_; }
    void SetInputPathAttr(const ResourceRef& value);
    void ChangeEffectOrder(const TagEffectPair& effect, unsigned newPos);

    const VariantVector& GetViewportsIndexesAttr() const { return viewportsIndexesAttr_; }
    void SetVewportsIndexesAttr(const VariantVector& value);

    void UpdateAll();
    void UpdateAutoExposure();
    void UpdateBloom();
    void UpdateBloomHDR();
    void UpdateBlur();
    void UpdateFXAA2();
    void UpdateFXAA3();
    void UpdateGammaCorrection();
    void UpdateTonemapping();
    void UpdateColorCorrection();
    void UpdateGreyScale();

    void SetAllEnabled(bool enable);

private:
    SharedPtr<RenderPath> effectsPath_;
    bool useAutoExposure_;
    float aeAdaptRate_;
    Vector2 aeLumRange_;
    float aeMiddleGrey_;
    bool useBloom_;
    float bloomThreshold_;
    Vector2 bloomMix_;
    bool useBloomHDR_;
    float bloomHDRThreshold_;
    Vector2 bloomHDRMix_;
    Vector2 bloomHDRDirH_;
    Vector2 bloomHDRDirV_;
    float bloomHDRBlurRadius_;
    float bloomHDRBlurSigma_;
    bool useBlur_;
    Vector2 blurDirV_;
    Vector2 blurDirH_;
    float blurRadius_;
    float blurSigma_;
    bool useFXAA2_;
    Vector3 FXAAParams_;
    bool useFXAA3_;
    unsigned FXAA3QualityPreset_;
    bool useGammaCorrection_;
    TonemapMode tonemapMode_;
    float tonemapExposureBias_;
    float tonemapMaxWhite_;
    bool useColorCorrection_;
    bool useGreyScale_;
    SharedPtr<Texture3D> lutTexture_;
    Vector<TagEffectPair> effectsOrder_;
    Renderer * renderer_;
    Vector<WeakPtr<Viewport>> viewports_;
    mutable VariantVector viewportsIndexesAttr_;
    ResourceRef inputPathAttr_;

    bool effectsOrderDirty_;
};