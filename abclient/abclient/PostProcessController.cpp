#include "stdafx.h"

#include "PostProcessController.h"

#include <Urho3D/Core/Context.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Graphics/RenderPath.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Resource/XMLFile.h>
#include <Urho3D/Graphics/Texture3D.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/IO/Log.h>

#include <algorithm>

static const StringVector viewportsStructureElementNames =
{
    "Number of viewports",
    "   viewport index:"
};

static const char* tonemapTypesNames[] =
{
    "TonemapReinhardEq3",
    "TonemapReinhardEq4",
    "TonemapUncharted2",
    "TonemapDisabled",
    nullptr
};


//tags
static const String& AUTOEXPOSURE_TAG = "AutoExposure";
static const String& BLOOM_TAG = "Bloom";
static const String& BLOOM_HDR_TAG = "BloomHDR";
static const String& BLUR_TAG = "Blur";
static const String& FXAA2_TAG = "FXAA2";
static const String& FXAA3_TAG = "FXAA3";
static const String& GAMMA_COR_TAG = "GammaCorrection";
static const String& COLOR_COR_TAG = "ColorCorrection";
static const String& GREY_SCALE_TAG = "GreyScale";

static const TagEffectPair& AUTOEXPOSURE_PAIR = TagEffectPair(AUTOEXPOSURE_TAG, "PostProcess/AutoExposure.xml");
static const TagEffectPair& GAMMA_CORRECTION_PAIR = TagEffectPair(GAMMA_COR_TAG, "PostProcess/GammaCorrection.xml");
static const TagEffectPair& BLOOM_PAIR = TagEffectPair(BLOOM_TAG, "PostProcess/Bloom.xml");
static const TagEffectPair& BLOOM_HDR_PAIR = TagEffectPair(BLOOM_HDR_TAG, "PostProcess/BloomHDR.xml");
static const TagEffectPair& BLUR_PAIR = TagEffectPair(BLUR_TAG, "PostProcess/Blur.xml");
static const TagEffectPair& FXAA2_PAIR = TagEffectPair(FXAA2_TAG, "PostProcess/FXAA2.xml");
static const TagEffectPair& FXAA3_PAIR = TagEffectPair(FXAA3_TAG, "PostProcess/FXAA3.xml");
static const TagEffectPair& COLOR_COR_PAIR = TagEffectPair(COLOR_COR_TAG, "PostProcess/ColorCorrection.xml");
static const TagEffectPair& TONEMAP_PAIR = TagEffectPair(tonemapTypesNames[0], "PostProcess/Tonemap.xml");
static const TagEffectPair& GREY_SCALE_PAIR = TagEffectPair(GREY_SCALE_TAG, "PostProcess/GreyScale.xml");

static const ResourceRef& DEF_LUT_TEXTURE = ResourceRef(Texture3D::GetTypeStatic(), "Textures/LUTIdentity.xml");

PostProcessController::PostProcessController(Context * context) :
    Component(context),
    lutTexture_(nullptr),
    inputPathAttr_(ResourceRef(XMLFile::GetTypeStatic()))
{
    renderer_ = GetSubsystem<Renderer>();
    viewportsIndexesAttr_.Push(0);
    effectsOrder_.Push(GREY_SCALE_PAIR);
    effectsOrder_.Push(AUTOEXPOSURE_PAIR);
    effectsOrder_.Push(GAMMA_CORRECTION_PAIR);
    effectsOrder_.Push(BLOOM_PAIR);
    effectsOrder_.Push(BLOOM_HDR_PAIR);
    effectsOrder_.Push(BLUR_PAIR);
    effectsOrder_.Push(TONEMAP_PAIR);
    effectsOrder_.Push(COLOR_COR_PAIR);
    effectsOrder_.Push(FXAA2_PAIR);
    effectsOrder_.Push(FXAA3_PAIR);
    BuildEffectsPath();
}

PostProcessController::~PostProcessController()
{
    ClearViewports();
}

void PostProcessController::RegisterObject(Context * context)
{
    context->RegisterFactory<PostProcessController>("Graphics");

    URHO3D_ACCESSOR_ATTRIBUTE("Input render path", GetInputPathAttr, SetInputPathAttr, ResourceRef, ResourceRef(XMLFile::GetTypeStatic()), AM_FILE);
    URHO3D_ACCESSOR_ATTRIBUTE("Viewports", GetViewportsIndexesAttr, SetVewportsIndexesAttr, VariantVector, Variant::emptyVariantVector, AM_FILE)
        .SetMetadata(AttributeMetadata::P_VECTOR_STRUCT_ELEMENTS, viewportsStructureElementNames);
    URHO3D_ACCESSOR_ATTRIBUTE("Auto exposure", IsUsingAutoExposure, SetUseAutoExposure, bool, false, AM_FILE);
    URHO3D_ACCESSOR_ATTRIBUTE("Adaptation rate", GetAutoExposureAdaptRate, SetAutoExposureAdaptRate, float, 0.6f, AM_FILE);
    URHO3D_ACCESSOR_ATTRIBUTE("Lum Range", GetAutoExposureLumRange, SetAutoExposureLumRange, Vector2, Vector2(0.01f, 1.0f), AM_FILE);
    URHO3D_ACCESSOR_ATTRIBUTE("Middle Grey", GetAutoExposureMidGrey, SetAutoExposureMidGrey, float, 0.6f, AM_FILE);
    URHO3D_ACCESSOR_ATTRIBUTE("Bloom", IsUsingBloom, SetUseBloom, bool, false, AM_FILE);
    URHO3D_ACCESSOR_ATTRIBUTE("Bloom threshold", GetBloomThreshold, SetBloomThreshold, float, 0.3f, AM_FILE);
    URHO3D_ACCESSOR_ATTRIBUTE("Bloom mix", GetBloomMix, SetBloomMix, Vector2, Vector2(0.9f, 0.4f), AM_FILE);
    URHO3D_ACCESSOR_ATTRIBUTE("Bloom HDR", IsUsingBloomHDR, SetUseBloomHDR, bool, false, AM_FILE);
    URHO3D_ACCESSOR_ATTRIBUTE("BloomHDR threshold", GetBloomHDRThreshold, SetBloomHDRThreshold, float, 0.8f, AM_FILE);
    URHO3D_ACCESSOR_ATTRIBUTE("BloomHDR mix", GetBloomHDRMix, SetBloomHDRMix, Vector2, Vector2(1.0f, 0.4f), AM_FILE);
    URHO3D_ACCESSOR_ATTRIBUTE("BloomHDR dir h", GetBloomHDRDirH, SetBloomHDRDirH, Vector2, Vector2(1.0f, 0.0f), AM_FILE);
    URHO3D_ACCESSOR_ATTRIBUTE("BloomHDR dir v", GetBloomHDRDirV, SetBloomHDRDirV, Vector2, Vector2(0.0f, 1.0f), AM_FILE);
    URHO3D_ACCESSOR_ATTRIBUTE("BloomHDR blur radius", GetBloomHDRBlurRadius, SetBloomHDRBlurRadius, float, 1.0f, AM_FILE);
    URHO3D_ACCESSOR_ATTRIBUTE("BloomHDR blur sigma", GetBloomHDRBlurSigma, SetBloomHDRBlurSigma, float, 2.0f, AM_FILE);
    URHO3D_ACCESSOR_ATTRIBUTE("Blur", IsUsingBlur, SetUseBlur, bool, false, AM_FILE);
    URHO3D_ACCESSOR_ATTRIBUTE("Blur dir h", GetBlurDirH, SetBlurDirH, Vector2, Vector2(1.0f, 0.0f), AM_FILE);
    URHO3D_ACCESSOR_ATTRIBUTE("Blur dir v", GetBlurDirV, SetBlurDirV, Vector2, Vector2(0.0f, 1.0f), AM_FILE);
    URHO3D_ACCESSOR_ATTRIBUTE("Blur radius", GetBlurRadius, SetBlurRadius, float, 2.0f, AM_FILE);
    URHO3D_ACCESSOR_ATTRIBUTE("Blur sigma", GetBlurSigma, SetBlurSigma, float, 2.0f, AM_FILE);
    URHO3D_ACCESSOR_ATTRIBUTE("FXAA2", IsUsingFXAA2, SetUseFXAA2, bool, false, AM_FILE);
    URHO3D_ACCESSOR_ATTRIBUTE("FXAA params", GetFXAAParams, SetFXAAParams, Vector3, Vector3(0.4f, 0.5f, 0.75f), AM_FILE);
    URHO3D_ACCESSOR_ATTRIBUTE("FXAA3", IsUsingFXAA3, SetUseFXAA3, bool, false, AM_FILE);
    URHO3D_ACCESSOR_ATTRIBUTE("FXAA3 quality preset", GetFXAA3QualityPreset, SetFXAA3QualityPreset, unsigned, 12, AM_FILE);
    URHO3D_ACCESSOR_ATTRIBUTE("Gamma Correction", IsUsingGammaCorrection, SetUseGammaCorrection, bool, false, AM_FILE);
    URHO3D_ENUM_ACCESSOR_ATTRIBUTE("Tonemap", GetTonemapMode, SetTonemapMode, TonemapMode, tonemapTypesNames, TONEMAP_DISABLED, AM_FILE);
    URHO3D_ACCESSOR_ATTRIBUTE("Tonemap exposure bias", GetTonemapExposureBias, SetTonemapExposureBias, float, 1.0f, AM_FILE);
    URHO3D_ACCESSOR_ATTRIBUTE("Tonemap max white", GetTonemapMaxWhite, SetTonemapMaxWhite, float, 1.0f, AM_FILE);
    URHO3D_ACCESSOR_ATTRIBUTE("Color Correction", IsUsingColorCorrection, SetUseColorCorrection, bool, false, AM_FILE);
    URHO3D_MIXED_ACCESSOR_ATTRIBUTE("Lut texture", GetLutTextureAttr, SetLutTextureAttr, ResourceRef, DEF_LUT_TEXTURE, AM_FILE);
    URHO3D_ACCESSOR_ATTRIBUTE("Grey scale", IsUsingGreyScale, SetUseGreyScale, bool, false, AM_FILE);

    URHO3D_ACCESSOR_ATTRIBUTE("Auto exposure order", GetAutoExposureOrder, SetAutoExposureOrder, unsigned, 0, AM_FILE);
    URHO3D_ACCESSOR_ATTRIBUTE("Bloom order", GetBloomOrder, SetBloomOrder, unsigned, 0, AM_FILE);
    URHO3D_ACCESSOR_ATTRIBUTE("Bloom HDR order", GetBloomHDROrder, SetBloomHDROrder, unsigned, 0, AM_FILE);
    URHO3D_ACCESSOR_ATTRIBUTE("Blur order", GetBlurOrder, SetBlurOrder, unsigned, 0, AM_FILE);
    URHO3D_ACCESSOR_ATTRIBUTE("Gamma Correction order", GetGammaCorrectionOrder, SetGammaCorrectionOrder, unsigned, 0, AM_FILE);
    URHO3D_ACCESSOR_ATTRIBUTE("Tonemap order", GetTonemapOrder, SetToneMapOrder, unsigned, 0, AM_FILE);
    URHO3D_ACCESSOR_ATTRIBUTE("Color Correction order", GetColorCorrectionOrder, SetColorCorrectionOrder, unsigned, 0, AM_FILE);
    URHO3D_ACCESSOR_ATTRIBUTE("FXAA2 order", GetFXAA2Order, SetFXAA2Order, unsigned, 0, AM_FILE);
    URHO3D_ACCESSOR_ATTRIBUTE("FXAA3 order", GetFXAA3Order, SetFXAA3Order, unsigned, 0, AM_FILE);
    URHO3D_ACCESSOR_ATTRIBUTE("Grey scale order", GetGreyScaleOrder, SetGreyScaleOrder, unsigned, 0, AM_FILE);
}

unsigned PostProcessController::GetAutoExposureOrder() const
{
    return effectsOrder_.IndexOf(AUTOEXPOSURE_PAIR);
}

void PostProcessController::SetAutoExposureOrder(unsigned val)
{
    if (val != GetAutoExposureOrder())
    {
        ChangeEffectOrder(AUTOEXPOSURE_PAIR, val);
        UpdateAutoExposure();
    }
}

void PostProcessController::SetUseAutoExposure(bool val)
{
    useAutoExposure_ = val;
    UpdateAutoExposure();
}

void PostProcessController::SetAutoExposureAdaptRate(float val)
{
    aeAdaptRate_ = val;
    UpdateAutoExposure();
}

void PostProcessController::SetAutoExposureLumRange(const Vector2 & val)
{
    aeLumRange_ = val;
    UpdateAutoExposure();
}

void PostProcessController::SetAutoExposureMidGrey(float val)
{
    aeMiddleGrey_ = val;
    UpdateAutoExposure();
}

unsigned PostProcessController::GetBloomOrder() const
{
    return effectsOrder_.IndexOf(BLOOM_PAIR);
}

void PostProcessController::SetBloomOrder(unsigned val)
{
    if (val != GetBloomOrder())
    {
        ChangeEffectOrder(BLOOM_PAIR, val);
        UpdateBloom();
    }
}

void PostProcessController::SetUseBloom(bool val)
{
    useBloom_ = val;
    UpdateBloom();
}

void PostProcessController::SetBloomThreshold(float val)
{
    bloomThreshold_ = val;
    UpdateBloom();
}

void PostProcessController::SetBloomMix(const Vector2 & val)
{
    bloomMix_ = val;
    UpdateBloom();
}

unsigned PostProcessController::GetBloomHDROrder() const
{
    return effectsOrder_.IndexOf(BLOOM_HDR_PAIR);
}

void PostProcessController::SetBloomHDROrder(unsigned value)
{
    if (value != GetBloomHDROrder())
    {
        ChangeEffectOrder(BLOOM_HDR_PAIR, value);
        UpdateBloomHDR();
    }
}

void PostProcessController::SetUseBloomHDR(bool val)
{
    useBloomHDR_ = val;
    UpdateBloomHDR();
}

void PostProcessController::SetBloomHDRThreshold(float val)
{
    bloomHDRThreshold_ = val;
    UpdateBloomHDR();
}

void PostProcessController::SetBloomHDRMix(const Vector2 & val)
{
    bloomHDRMix_ = val;
    UpdateBloomHDR();
}

void PostProcessController::SetBloomHDRDirH(const Vector2 & val)
{
    bloomHDRDirH_ = val;
    UpdateBloomHDR();
}

void PostProcessController::SetBloomHDRDirV(const Vector2 & val)
{
    bloomHDRDirV_ = val;
    UpdateBloomHDR();
}

void PostProcessController::SetBloomHDRBlurRadius(float val)
{
    bloomHDRBlurRadius_ = val;
    UpdateBloomHDR();
}

void PostProcessController::SetBloomHDRBlurSigma(float val)
{
    bloomHDRBlurSigma_ = val;
    UpdateBloomHDR();
}

unsigned PostProcessController::GetBlurOrder() const
{
    return effectsOrder_.IndexOf(BLUR_PAIR);
}

void PostProcessController::SetBlurOrder(unsigned value)
{
    if (value != GetBlurOrder())
    {
        ChangeEffectOrder(BLUR_PAIR, value);
        UpdateBloomHDR();
    }
}

void PostProcessController::SetUseBlur(bool val)
{
    useBlur_ = val;
    UpdateBlur();
}

void PostProcessController::SetBlurDirH(const Vector2 & val)
{
    blurDirH_ = val;
    UpdateBlur();
}

void PostProcessController::SetBlurDirV(const Vector2 & val)
{
    blurDirV_ = val;
    UpdateBlur();
}

void PostProcessController::SetBlurRadius(float val)
{
    blurRadius_ = val;
    UpdateBlur();
}

void PostProcessController::SetBlurSigma(float val)
{
    blurSigma_ = val;
    UpdateBlur();
}

void PostProcessController::SetUseFXAA2(bool val)
{
    useFXAA2_ = val;
    UpdateFXAA2();
}

unsigned PostProcessController::GetFXAA2Order() const
{
    return effectsOrder_.IndexOf(FXAA2_PAIR);
}

void PostProcessController::SetFXAA2Order(unsigned val)
{
    if (val != GetFXAA2Order())
    {
        ChangeEffectOrder(FXAA2_PAIR, val);
        UpdateFXAA2();
    }
}

void PostProcessController::SetUseFXAA3(bool val)
{
    useFXAA3_ = val;
    UpdateFXAA3();
}

unsigned PostProcessController::GetFXAA3Order() const
{
    return effectsOrder_.IndexOf(FXAA3_PAIR);
}

void PostProcessController::SetFXAA3Order(unsigned val)
{
    if (val != GetFXAA3Order())
    {
        ChangeEffectOrder(FXAA3_PAIR, val);
        UpdateFXAA3();
    }
}

void PostProcessController::SetFXAA3QualityPreset(unsigned val)
{
    if (val == FXAA3QualityPreset_)
        return;
    if (val < 20)
        FXAA3QualityPreset_ = Clamp<unsigned>(val, 10, 15);
    else if (val >= 20 && val < 39)
        FXAA3QualityPreset_ = Clamp<unsigned>(val, 20, 29);
    else
        FXAA3QualityPreset_ = 39;
    UpdateFXAA3();
}

void PostProcessController::SetFXAAParams(const Vector3 & val)
{
    FXAAParams_ = val;
    UpdateFXAA2();
}

void PostProcessController::SetUseGammaCorrection(bool val)
{
    useGammaCorrection_ = val;
    UpdateGammaCorrection();
}

unsigned PostProcessController::GetGammaCorrectionOrder() const
{
    return effectsOrder_.IndexOf(GAMMA_CORRECTION_PAIR);
}

void PostProcessController::SetGammaCorrectionOrder(unsigned val)
{
    if (val != GetGammaCorrectionOrder())
    {
        ChangeEffectOrder(GAMMA_CORRECTION_PAIR, val);
        UpdateGammaCorrection();
    }
}

void PostProcessController::SetTonemapMode(TonemapMode val)
{
    tonemapMode_ = val;
    UpdateTonemapping();
}

unsigned PostProcessController::GetTonemapOrder() const
{
    return effectsOrder_.IndexOf(TONEMAP_PAIR);
}

void PostProcessController::SetToneMapOrder(unsigned val)
{
    if (val != GetTonemapOrder())
    {
        ChangeEffectOrder(TONEMAP_PAIR, val);
        UpdateTonemapping();
    }
}

void PostProcessController::SetTonemapExposureBias(float val)
{
    tonemapExposureBias_ = Clamp(val, 0.0f, 10.0f);
    UpdateTonemapping();
}

void PostProcessController::SetTonemapMaxWhite(float val)
{
    tonemapMaxWhite_ = Clamp(val, 0.0f, 10.0f);
    UpdateTonemapping();
}

void PostProcessController::SetUseColorCorrection(bool val)
{
    useColorCorrection_ = val;
    UpdateColorCorrection();
}

unsigned PostProcessController::GetColorCorrectionOrder() const
{
    return effectsOrder_.IndexOf(COLOR_COR_PAIR);
}

void PostProcessController::SetColorCorrectionOrder(unsigned val)
{
    if (val != GetColorCorrectionOrder())
    {
        ChangeEffectOrder(COLOR_COR_PAIR, val);
        UpdateColorCorrection();
    }
}

void PostProcessController::SetUseGreyScale(bool val)
{
    useGreyScale_ = val;
    UpdateGreyScale();
}

unsigned PostProcessController::GetGreyScaleOrder() const
{
    return effectsOrder_.IndexOf(GREY_SCALE_PAIR);
}

void PostProcessController::SetGreyScaleOrder(unsigned val)
{
    if (val != GetGreyScaleOrder())
    {
        ChangeEffectOrder(GREY_SCALE_PAIR, val);
        UpdateGreyScale();
    }
}

Texture3D * PostProcessController::GetLutTexture()
{
    return lutTexture_ ? lutTexture_ : GetSubsystem<ResourceCache>()->GetResource<Texture3D>(DEF_LUT_TEXTURE.name_);
}

void PostProcessController::SetLutTexture(Texture3D * val)
{
    lutTexture_ = val;
    UpdateColorCorrection();
}

ResourceRef PostProcessController::GetLutTextureAttr() const
{
    return lutTexture_ ? GetResourceRef(lutTexture_, Texture3D::GetTypeStatic()) : DEF_LUT_TEXTURE;
}

void PostProcessController::SetLutTextureAttr(const ResourceRef & val)
{
    SetLutTexture(GetSubsystem<ResourceCache>()->GetResource<Texture3D>(val.name_));
}

void PostProcessController::ClearViewports()
{
    //prevent from crashing if destructor was called when application is being closed
    if (!GetSubsystem<Renderer>())
        return;
    for (auto vp : viewports_)
    {
        //vieport may have been removed by now
        if (vp)
        {
            vp->SetRenderPath(renderer_->GetDefaultRenderPath());
        }
    }
    viewports_.Clear();
}

bool PostProcessController::RemoveViewport(Viewport * target)
{
    if (target && viewports_.Remove(WeakPtr<Viewport>(target)))
    {
        target->SetRenderPath(renderer_->GetDefaultRenderPath());
        return true;
    }
    return false;
}

void PostProcessController::AddViewport(Viewport * target, bool clearList)
{
    if (clearList)
        ClearViewports();
    if (target)
    {
        target->SetRenderPath(effectsPath_);
        viewports_.Push(WeakPtr<Viewport>(target));
    }
}

bool PostProcessController::RemoveViewportIndex(unsigned index, bool removeViewport)
{
    if (viewportsIndexesAttr_.Remove(index))
    {
        if (removeViewport)
            RemoveViewport(renderer_->GetViewport(index));
        return true;
    }
    return false;
}

void PostProcessController::AddViewportIndex(unsigned index, bool clearList)
{
    if (clearList)
    {
        viewportsIndexesAttr_.Clear();
        viewportsIndexesAttr_.Push(1);
    }
    Viewport* vp = renderer_->GetViewport(index);
    if (vp && !viewports_.Contains(WeakPtr<Viewport>(vp)))
    {
        AddViewport(WeakPtr<Viewport>(vp), clearList);
    }
    viewportsIndexesAttr_.Push(index);
}

RenderPath * PostProcessController::BuildEffectsPath()
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    effectsPath_ = SharedPtr<RenderPath>(new RenderPath());
    XMLFile* file = cache->GetResource<XMLFile>(inputPathAttr_.name_);
    if (!effectsPath_->Load(file))
    {
        effectsPath_ = renderer_->GetDefaultRenderPath()->Clone();
    }
    for (auto pair : effectsOrder_)
    {
//        if (!effectsPath_->IsAdded(pair.first_))
//        {
            effectsPath_->Append(cache->GetResource<XMLFile>(pair.second_));
//        }
    }
    for (auto vp : viewports_)
    {
        //viewport may have been removed by now
        if (vp)
            vp->SetRenderPath(effectsPath_);
        else
            viewports_.Remove(vp);
    }
    effectsOrderDirty_ = false;
    UpdateAll();
    return effectsPath_;
}

RenderPath * PostProcessController::GetEffectsPath()
{
    return effectsOrderDirty_ ? BuildEffectsPath() : effectsPath_;
}

void PostProcessController::SetInputPathAttr(const ResourceRef & value)
{
    inputPathAttr_ = value;
    effectsOrderDirty_ = true;
    UpdateAll();
}

void PostProcessController::ChangeEffectOrder(const TagEffectPair & effect, unsigned newPos)
{
    if (newPos >= effectsOrder_.Size())
        newPos = effectsOrder_.Size() - 1;

    auto iter1 = effectsOrder_.Find(effect);
    auto iter2 = effectsOrder_.Find(effectsOrder_[newPos]);
    std::iter_swap(iter1, iter2);
    effectsOrderDirty_ = true;
}

void PostProcessController::UpdateAll()
{
    if (effectsOrderDirty_)
    {
        BuildEffectsPath();
    }
    else
    {
        UpdateAutoExposure();
        UpdateBloom();
        UpdateBloomHDR();
        UpdateBlur();
        UpdateFXAA2();
        UpdateColorCorrection();
        UpdateGammaCorrection();
        UpdateTonemapping();
        UpdateFXAA2();
        UpdateFXAA3();
        UpdateGreyScale();
    }
}

void PostProcessController::SetVewportsIndexesAttr(const VariantVector & value)
{
    if (value.Size())
    {
        viewportsIndexesAttr_.Clear();
        ClearViewports();
        unsigned index = 0;
        unsigned numViewports = index < value.Size() ? value[index++].GetUInt() : 0;
        if (numViewports > M_MAX_INT)
            numViewports = 0;
        viewportsIndexesAttr_.Push(numViewports);
        while (numViewports--)
        {
            if (index < value.Size())
            {
                AddViewportIndex(value[index++].GetUInt(), false);
            }
            else
            {
                //add high index so it won't override other controllers before user decide to do it
                viewportsIndexesAttr_.Push(M_MAX_INT);
            }
        }
    }
}

void PostProcessController::UpdateAutoExposure()
{
    if (effectsOrderDirty_)
        BuildEffectsPath();
    else
    {
        effectsPath_->SetEnabled(AUTOEXPOSURE_TAG, useAutoExposure_);
        if (useAutoExposure_)
        {
            effectsPath_->SetShaderParameter("AutoExposureAdaptRate", aeAdaptRate_);
            effectsPath_->SetShaderParameter("AutoExposureLumRange", aeLumRange_);
            effectsPath_->SetShaderParameter("AutoExposureMiddleGrey", aeMiddleGrey_);
        }

    }
}

void PostProcessController::UpdateBloom()
{
    if (effectsOrderDirty_)
        BuildEffectsPath();
    else
    {
        effectsPath_->SetEnabled(BLOOM_TAG, useBloom_);
        if (useBloom_)
        {
            effectsPath_->SetShaderParameter("BloomThreshold", bloomThreshold_);
            effectsPath_->SetShaderParameter("BloomMix", bloomMix_);
        }
    }
}

void PostProcessController::UpdateBloomHDR()
{
    if (effectsOrderDirty_)
        BuildEffectsPath();
    else
    {
        effectsPath_->SetEnabled(BLOOM_HDR_TAG, useBloomHDR_);
        if (useBloomHDR_)
        {
            effectsPath_->SetShaderParameter("BloomHDRThreshold", bloomHDRThreshold_);
            effectsPath_->SetShaderParameter("BloomHDRMix", bloomHDRMix_);
            effectsPath_->SetShaderParameter("BloomHDRBlurRadius", bloomHDRBlurRadius_);
            effectsPath_->SetShaderParameter("BloomHDRBlurSigma", bloomHDRBlurSigma_);
            for (auto& c : effectsPath_->commands_)
            {
                if (c.tag_ == BLOOM_HDR_TAG && c.vertexShaderName_ == BLOOM_HDR_TAG)
                {
                    for (auto& o : c.outputs_)
                    {
                        if (o.first_.Contains("blur"))
                            c.SetShaderParameter("BloomHDRBlurDir", bloomHDRDirH_);
                        else if (o.first_.Contains("bright"))
                            c.SetShaderParameter("BloomHDRBlurDir", bloomHDRDirV_);
                    }
                }
            }
        }
    }
}

void PostProcessController::UpdateBlur()
{
    if (effectsOrderDirty_)
        BuildEffectsPath();
    else
    {
        effectsPath_->SetEnabled(BLUR_TAG, useBlur_);
        if (useBlur_)
        {
            effectsPath_->SetShaderParameter("BlurRadius", blurRadius_);
            effectsPath_->SetShaderParameter("BlurSigma", blurSigma_);
            for (auto& c : effectsPath_->commands_)
            {
                if (c.tag_ == BLUR_TAG && c.vertexShaderName_ == BLUR_TAG)
                {
                    for (auto& o : c.outputs_)
                    {
                        if (o.first_ == "blurv")
                            c.SetShaderParameter("BlurDir", blurDirV_);
                        else if (o.first_ == "blurh")
                            c.SetShaderParameter("BlurDir", blurDirH_);
                    }
                }
            }
        }
    }
}

void PostProcessController::UpdateFXAA2()
{
    if (effectsOrderDirty_)
        BuildEffectsPath();
    else
    {
        effectsPath_->SetEnabled(FXAA2_TAG, useFXAA2_);
        if (useFXAA2_)
        {
            SetUseFXAA3(false);
            effectsPath_->SetShaderParameter("FXAAParams", FXAAParams_);
        }
    }
}

void PostProcessController::UpdateFXAA3()
{
    if (effectsOrderDirty_)
        BuildEffectsPath();
    else
    {
        effectsPath_->SetEnabled(FXAA3_TAG, useFXAA3_);
        if (useFXAA3_)
        {
            SetUseFXAA2(false);
            for (auto& c : effectsPath_->commands_)
            {
                if (c.tag_ == FXAA3_TAG && c.pixelShaderName_ == FXAA3_TAG)
                {
                    c.pixelShaderDefines_ = "FXAA_QUALITY_PRESET=" + String(FXAA3QualityPreset_);
                }
            }
        }
    }
}

void PostProcessController::UpdateGammaCorrection()
{
    if (effectsOrderDirty_)
        BuildEffectsPath();
    else
    {
        effectsPath_->SetEnabled(GAMMA_COR_TAG, useGammaCorrection_);
    }
}

void PostProcessController::UpdateTonemapping()
{
    if (effectsOrderDirty_)
        BuildEffectsPath();
    else
    {
        bool enabled = tonemapMode_ != TONEMAP_DISABLED;
        if (!enabled)
        {
            for (unsigned i = 0; i < 3; ++i)
            {
                effectsPath_->SetEnabled(tonemapTypesNames[i], false);
            }
            return;
        }
        const String& tag = tonemapTypesNames[tonemapMode_];
        for (unsigned i = 0; i < 3; ++i)
        {
            const String& t = tonemapTypesNames[i];
            effectsPath_->SetEnabled(t, t == tag ? true : false);
        }
        effectsPath_->SetShaderParameter("TonemapExposureBias", Variant(tonemapExposureBias_));
        effectsPath_->SetShaderParameter("TonemapMaxWhite", Variant(tonemapMaxWhite_));
    }
}

void PostProcessController::UpdateColorCorrection()
{
    if (effectsOrderDirty_)
        BuildEffectsPath();
    else
    {
        lutTexture_ = GetLutTexture();
        if (lutTexture_)
        {
            effectsPath_->SetEnabled(COLOR_COR_TAG, useColorCorrection_);
            if (useColorCorrection_)
            {
                for (auto& command : effectsPath_->commands_)
                {
                    if (command.tag_ == COLOR_COR_TAG && command.enabled_)
                    {
                        command.SetTextureName(TU_VOLUMEMAP, lutTexture_->GetName());
                    }
                }
            }
        }
    }
}

void PostProcessController::UpdateGreyScale()
{
    if (effectsOrderDirty_)
        BuildEffectsPath();
    else
    {
        effectsPath_->SetEnabled(GREY_SCALE_TAG, useGreyScale_);
    }
}

void PostProcessController::SetAllEnabled(bool enable)
{
    //tonemap type must set manually
    SetTonemapMode(enable ? REINHARDEQ3 : TONEMAP_DISABLED);
    SetUseAutoExposure(enable);
    SetUseBloom(enable);
    SetUseBloomHDR(enable);
    SetUseBlur(enable);
    SetUseColorCorrection(enable);
    SetUseColorCorrection(enable);
}
