#include "stdafx.h"
#include "CharCreateLevel.h"
#include "AbEvents.h"
#include "Structs.h"
#include "FwClient.h"
#include <AB/Entities/Character.h>
#include <AB/Entities/Limits.h>

#include <Urho3D/DebugNew.h>

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

    nameEdit_ = dynamic_cast<LineEdit*>(uiRoot_->GetChild("CharacterNameEdit", true));
    nameEdit_->SetMaxLength(AB::Entities::Limits::MAX_CHARACTER_NAME);
    professionDropdown_ = dynamic_cast<DropDownList*>(uiRoot_->GetChild("ProfessionDropDown", true));
    sexDropdown_ = dynamic_cast<DropDownList*>(uiRoot_->GetChild("GenderDropDown", true));
    createButton_ = dynamic_cast<Button*>(uiRoot_->GetChild("CreateButton", true));
    cancelButton_ = dynamic_cast<Button*>(uiRoot_->GetChild("CancelButton", true));
    SubscribeToEvent(createButton_, E_RELEASED, URHO3D_HANDLER(CharCreateLevel, HandleCreateClicked));
    SubscribeToEvent(cancelButton_, E_RELEASED, URHO3D_HANDLER(CharCreateLevel, HandleCancelClicked));

    professionDropdown_->GetPopup()->SetWidth(professionDropdown_->GetWidth());
    professionDropdown_->AddItem(CreateDropdownItem("(Select primary Profession)", ""));
    FwClient* client = GetSubsystem<FwClient>();
    // Load local file or download from file server
    const auto& profs = client->GetProfessions();
    for (const auto& prof : profs)
    {
        if (prof.second.index != 0)
            professionDropdown_->AddItem(CreateDropdownItem(String(prof.second.name.c_str()), String(prof.first.c_str())));
    }

    sexDropdown_->GetPopup()->SetWidth(sexDropdown_->GetWidth());
    sexDropdown_->AddItem(CreateDropdownItem("(Select Gender)", static_cast<uint32_t>(AB::Entities::CharacterSexUnknown)));
    sexDropdown_->AddItem(CreateDropdownItem("Female", static_cast<uint32_t>(AB::Entities::CharacterSexFemale)));
    sexDropdown_->AddItem(CreateDropdownItem("Male", static_cast<uint32_t>(AB::Entities::CharacterSexMale)));

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
    if (name.Length() < 6)
    {
        ShowError("Names must have at least six characters.");
        nameEdit_->SetFocus(true);
        return;
    }
    if (name.Length() > 20)
    {
        ShowError("The name is too long. Max 20 characters allowed.");
        nameEdit_->SetFocus(true);
        return;
    }
    Text* profTxt = dynamic_cast<Text*>(professionDropdown_->GetSelectedItem());
    String prof = profTxt->GetVar("String Value").GetString();
    if (prof.Empty())
    {
        ShowError("Please select the primary profession of your character.");
        return;
    }

    Text* sexTxt = dynamic_cast<Text*>(sexDropdown_->GetSelectedItem());
    uint32_t sex = sexTxt->GetVar("Int Value").GetInt();
    if (sex == 0 || sex > AB::Entities::CharacterSexMale)
    {
        ShowError("Please select the gender of your character.");
        return;
    }

    FwClient* client = GetSubsystem<FwClient>();
    const auto& profs = client->GetProfessions();
    AB::Entities::CharacterSex _sex = static_cast<AB::Entities::CharacterSex>(sex);
    uint32_t modelIndex = 0;
    auto profIt = profs.find(std::string(prof.CString()));
    if (profIt != profs.end())
    {
        if (_sex == AB::Entities::CharacterSexFemale)
            modelIndex = (*profIt).second.modelIndexFemale;
        else
            modelIndex = (*profIt).second.modelIndexMale;
    }
    if (modelIndex == 0)
        modelIndex = 1;

    client->CreatePlayer(name, prof, modelIndex, _sex, true);
}

void CharCreateLevel::DoCancel()
{
    VariantMap& e = GetEventDataMap();
    using namespace AbEvents::SetLevel;
    e[P_NAME] = "CharSelectLevel";
    SendEvent(AbEvents::E_SETLEVEL, e);
}

void CharCreateLevel::CreateScene()
{
    BaseLevel::CreateScene();
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    XMLFile *sceneFile = cache->GetResource<XMLFile>("Scenes/CreateCharacter.xml");
    scene_->LoadXML(sceneFile->GetRoot());
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
