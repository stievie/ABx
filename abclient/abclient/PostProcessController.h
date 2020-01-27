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
using TagEffectPair = Pair<String, String>;

class PostProcessController : public Component
{
    URHO3D_OBJECT(PostProcessController, Component)
public:
    PostProcessController(Context* context);
    ~PostProcessController() override;

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
    bool useAutoExposure_{ false };
    float aeAdaptRate_{ 0.6f };
    Vector2 aeLumRange_{ 0.01f, 1.0f };
    float aeMiddleGrey_{ 0.6f };
    bool useBloom_{ false };
    float bloomThreshold_{ 0.3f };
    Vector2 bloomMix_{ 0.9f, 0.4f };
    bool useBloomHDR_{ false };
    float bloomHDRThreshold_{ 0.8f };
    Vector2 bloomHDRMix_{ 1.0f, 0.4f };
    Vector2 bloomHDRDirH_{ 1.0f, 0.0f };
    Vector2 bloomHDRDirV_{ 0.0f, 1.0f };
    float bloomHDRBlurRadius_{ 1.0f };
    float bloomHDRBlurSigma_{ 2.0f };
    bool useBlur_{ false };
    Vector2 blurDirV_{ 0.0f , 1.0f };
    Vector2 blurDirH_{ 1.0f, 0.0f };
    float blurRadius_{ 2.0f };
    float blurSigma_{ 2.0f };
    bool useFXAA2_{ false };
    Vector3 FXAAParams_{ 0.4f, 0.5f, 0.75f };
    bool useFXAA3_{ false };
    unsigned FXAA3QualityPreset_{ 12 };
    bool useGammaCorrection_{ false };
    TonemapMode tonemapMode_{ TONEMAP_DISABLED };
    float tonemapExposureBias_{ 1.0f };
    float tonemapMaxWhite_{ 1.0f };
    bool useColorCorrection_{ false };
    bool useGreyScale_{ false };
    SharedPtr<Texture3D> lutTexture_;
    Vector<TagEffectPair> effectsOrder_;
    Renderer* renderer_;
    Vector<WeakPtr<Viewport>> viewports_;
    mutable VariantVector viewportsIndexesAttr_;
    ResourceRef inputPathAttr_;

    bool effectsOrderDirty_{ true };
};
