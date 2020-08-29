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


#include "CharCreateLevel.h"
#include "Structs.h"
#include "FwClient.h"
#include <AB/Entities/Character.h>
#include <AB/Entities/Limits.h>
#include "AudioManager.h"
#include "SkillManager.h"
#include <AB/CommonConfig.h>

//#include <Urho3D/DebugNew.h>

CharCreateLevel::CharCreateLevel(Context* context) :
    BaseLevel(context)
{
    // Create the scene content
    CreateScene();
    CreateCamera();

    // Create the UI content
    CreateUI();
    CreateLogo();

    // Subscribe to global events for camera movement
    SubscribeToEvents();
}

void CharCreateLevel::CreateCamera()
{
    cameraNode_ = scene_->GetChild("CameraNode");
    if (!cameraNode_)
    {
        cameraNode_ = scene_->CreateChild("CameraNode");
        Camera* camera = cameraNode_->CreateComponent<Camera>();
        camera->SetFarClip(300.0f);
    }
    SetupViewport();
}

void CharCreateLevel::SubscribeToEvents()
{
    BaseLevel::SubscribeToEvents();
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(CharCreateLevel, HandleUpdate));
}

void CharCreateLevel::CreateUI()
{
    uiRoot_->RemoveAllChildren();
    BaseLevel::CreateUI();

    ResourceCache* cache = GetSubsystem<ResourceCache>();
    XMLFile *chatFile = cache->GetResource<XMLFile>("UI/CreateCharacterWindow.xml");
    uiRoot_->LoadChildXML(chatFile->GetRoot(), nullptr);

    nameEdit_ = uiRoot_->GetChildStaticCast<LineEdit>("CharacterNameEdit", true);
    nameEdit_->SetMaxLength(AB::Entities::Limits::MAX_CHARACTER_NAME);
    professionDropdown_ = uiRoot_->GetChildStaticCast<DropDownList>("ProfessionDropDown", true);
    sexDropdown_ = uiRoot_->GetChildStaticCast<DropDownList>("GenderDropDown", true);
    pvpCheckbox_ = uiRoot_->GetChildStaticCast<CheckBox>("PvpCharacterCheckbox", true);
    createButton_ = uiRoot_->GetChildStaticCast<Button>("CreateButton", true);
    cancelButton_ = uiRoot_->GetChildStaticCast<Button>("CancelButton", true);
    SubscribeToEvent(createButton_, E_RELEASED, URHO3D_HANDLER(CharCreateLevel, HandleCreateClicked));
    SubscribeToEvent(cancelButton_, E_RELEASED, URHO3D_HANDLER(CharCreateLevel, HandleCancelClicked));

    professionDropdown_->GetPopup()->SetWidth(professionDropdown_->GetWidth());
    professionDropdown_->AddItem(CreateDropdownItem("(Select primary Profession)", ""));
    // Load local file or download from file server
    const auto& profs = GetSubsystem<SkillManager>()->GetProfessions();
    for (const auto& prof : profs)
    {
        if (prof.second.index != 0)
            professionDropdown_->AddItem(CreateDropdownItem(String(prof.second.name.c_str()), String(prof.first.c_str())));
    }

    sexDropdown_->GetPopup()->SetWidth(sexDropdown_->GetWidth());
    sexDropdown_->AddItem(CreateDropdownItem("(Select Gender)", static_cast<uint32_t>(AB::Entities::CharacterSex::Unknown)));
    sexDropdown_->AddItem(CreateDropdownItem("Female", static_cast<uint32_t>(AB::Entities::CharacterSex::Female)));
    sexDropdown_->AddItem(CreateDropdownItem("Male", static_cast<uint32_t>(AB::Entities::CharacterSex::Male)));

    nameEdit_->SetFocus(true);
}

Text* CharCreateLevel::CreateDropdownItem(const String& text, uint32_t value)
{
    Text* result = new Text(context_);
    result->SetText(text);
    result->SetVar("Int Value", value);
    result->SetStyle("DropDownItemEnumText");
    return result;
}

Text* CharCreateLevel::CreateDropdownItem(const String& text, const String& value)
{
    Text* result = new Text(context_);
    result->SetText(text);
    result->SetVar("String Value", value);
    result->SetStyle("DropDownItemEnumText");
    return result;
}

void CharCreateLevel::DoCreateCharacter()
{
    String name = nameEdit_->GetText().Trimmed();
    if (name.Empty())
    {
        ShowError("Please enter a name for your character.");
        nameEdit_->SetFocus(true);
        return;
    }
    if (name.Length() < CHARACTER_NAME_NIM)
    {
        ShowError("Names must have at least six characters.");
        nameEdit_->SetFocus(true);
        return;
    }
    if (name.Length() > CHARACTER_NAME_MAX)
    {
        ShowError("The name is too long. Max 20 characters allowed.");
        nameEdit_->SetFocus(true);
        return;
    }
    String restircted(RESTRICTED_NAME_CHARS);
    for (unsigned i = 0; i < restircted.Length(); ++i)
    {
        if (name.Contains(restircted.At(i)))
        {
            ShowError("The name contains invalid characters. A name must not contain " + restircted);
            nameEdit_->SetFocus(true);
            return;
        }
    }
    Text* profTxt = static_cast<Text*>(professionDropdown_->GetSelectedItem());
    String prof = profTxt->GetVar("String Value").GetString();
    if (prof.Empty())
    {
        ShowError("Please select the primary profession of your character.");
        return;
    }

    Text* sexTxt = static_cast<Text*>(sexDropdown_->GetSelectedItem());
    uint32_t sex = sexTxt->GetVar("Int Value").GetUInt();
    if (sex == 0 || sex > static_cast<uint32_t>(AB::Entities::CharacterSex::Male))
    {
        ShowError("Please select the gender of your character.");
        return;
    }
    bool pvp = pvpCheckbox_->IsChecked();

    FwClient* client = GetSubsystem<FwClient>();
    SkillManager* sm = GetSubsystem<SkillManager>();
    const auto& profs = sm->GetProfessions();
    AB::Entities::CharacterSex _sex = static_cast<AB::Entities::CharacterSex>(sex);
    uint32_t modelIndex = 0;
    auto profIt = profs.find(std::string(prof.CString()));
    if (profIt != profs.end())
    {
        if (_sex == AB::Entities::CharacterSex::Female)
            modelIndex = (*profIt).second.modelIndexFemale;
        else
            modelIndex = (*profIt).second.modelIndexMale;
    }
    if (modelIndex == 0)
        modelIndex = 1;

    client->CreatePlayer(name, prof, modelIndex, _sex, pvp);
}

void CharCreateLevel::DoCancel()
{
    VariantMap& e = GetEventDataMap();
    using namespace Events::SetLevel;
    e[P_NAME] = "CharSelectLevel";
    SendEvent(Events::E_SETLEVEL, e);
}

void CharCreateLevel::CreateScene()
{
    BaseLevel::CreateScene();
    LoadScene("Scenes/CreateCharacter.xml");

    using namespace Events::AudioPlayMapMusic;
    VariantMap& e = GetEventDataMap();
    e[P_MAPUUID] = "CreateCharacter";
    SendEvent(Events::E_AUDIOPLAYMAPMUSIC, e);
}

void CharCreateLevel::HandleUpdate(StringHash, VariantMap& eventData)
{
    using namespace Update;

    // Take the frame time step, which is stored as a float
    float timeStep = eventData[P_TIMESTEP].GetFloat();
    Quaternion rot;
    rot.FromAngleAxis(timeStep, Vector3(0.0f, 1.0f, 0.0f));
    cameraNode_->Rotate(rot);
}

void CharCreateLevel::HandleCreateClicked(StringHash, VariantMap&)
{
    DoCreateCharacter();
}

void CharCreateLevel::HandleCancelClicked(StringHash, VariantMap&)
{
    DoCancel();
}
