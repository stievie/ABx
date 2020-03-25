/**
 * Copyright 2020 Stefan Ascher
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

#include "stdafx.h"
#include "FilePicker.h"
#include <AB/CommonConfig.h>

static bool CompareEntries(const FileSelectorEntry& lhs, const FileSelectorEntry& rhs)
{
    if (lhs.directory_ && !rhs.directory_)
        return true;
    if (!lhs.directory_ && rhs.directory_)
        return false;
#ifdef AB_WINDOWS
    return lhs.name_.Compare(rhs.name_, false) < 0;
#elif AB_UNIX
    return lhs.name_.Compare(rhs.name_, true) < 0;
#endif
}

FilePicker::FilePicker(Context* context, const String& root, Mode mode) :
    DialogWindow(context),
    root_(AddTrailingSlash(root)),
    path_(root_),
    mode_(mode)
{
    LoadLayout("UI/FilePicker.xml");
    SetStyleAuto();

    SetSize(430, 400);
    SetMinSize(430, 400);
    SetMaxSize(430, 400);
    SetMovable(true);
    SetFocusMode(FM_FOCUSABLE);

    auto* caption = GetChildDynamicCast<Text>("Caption", true);
    if (mode_ == Mode::Load)
        caption->SetText("Load file");
    else
        caption->SetText("Save file");

    SubscribeEvents();

    auto* newFolder = GetChildDynamicCast<Button>("NewFolderButton", true);
    newFolder->SetVisible(false);

    auto* okButton = GetChildDynamicCast<Button>("OkButton", true);
    SubscribeToEvent(okButton, E_RELEASED, URHO3D_HANDLER(FilePicker, HandleOkClicked));
    auto* cancelButton = GetChildDynamicCast<Button>("CancelButton", true);
    SubscribeToEvent(cancelButton, E_RELEASED, URHO3D_HANDLER(FilePicker, HandleCancelClicked));

    fileList_ = GetChildDynamicCast<ListView>("FilesList", true);
    SubscribeToEvent(fileList_, E_ITEMSELECTED, URHO3D_HANDLER(FilePicker, HandleFileSelected));
    SubscribeToEvent(fileList_, E_ITEMDOUBLECLICKED, URHO3D_HANDLER(FilePicker, HandleFileDoubleClicked));
    fileNameEdit_ = GetChildDynamicCast<LineEdit>("FilenameEdit", true);
    SetBringToBack(false);
    uiRoot_->AddChild(this);
    SetVisible(true);
    MakeModal();
    Center();
    SetPriority(200);
    BringToFront();
    SetPath(root_);
    fileNameEdit_->SetFocus(true);
}

void FilePicker::SetPath(const String& path)
{
    path_ = path;
    ScanPath();
}

void FilePicker::HandleCancelClicked(StringHash, VariantMap&)
{
    Close();
}

void FilePicker::HandleOkClicked(StringHash, VariantMap&)
{
    if (mode_ == Mode::Save)
    {
        const String& fn = fileNameEdit_->GetText();
        if (!fn.Empty())
        {
            String fileName = AddTrailingSlash(path_) + fn;
            using namespace FilePicked;

            VariantMap& eventData = GetEventDataMap();
            eventData[P_FILENAME] = fileName;
            SendEvent(E_FILEPICKED, eventData);
            Close();
            return;
        }
    }
    EnterFile();
}

void FilePicker::HandleFileSelected(StringHash, VariantMap&)
{
    unsigned index = fileList_->GetSelection();
    if (index >= fileEntries_.Size())
        return;
    if (!fileEntries_[index].directory_)
        fileNameEdit_->SetText(fileEntries_[index].name_);
}

void FilePicker::HandleFileDoubleClicked(StringHash, VariantMap& eventData)
{
    if (eventData[ItemDoubleClicked::P_BUTTON] == MOUSEB_LEFT)
        EnterFile();
}

void FilePicker::ScanPath()
{
    auto* fileSystem = GetSubsystem<FileSystem>();

    fileList_->RemoveAllItems();
    fileEntries_.Clear();

    Vector<String> directories;
    Vector<String> files;
    fileSystem->ScanDir(directories, path_, "*", SCAN_DIRS, false);
    fileSystem->ScanDir(files, path_, "*", SCAN_FILES, false);

    for (unsigned i = 0; i < directories.Size(); ++i)
    {
        FileSelectorEntry newEntry;
        if (directories[i] == ".")
            continue;
        if (directories[i] == "..")
        {
#ifdef AB_WINDOWS
            if (path_.Compare(root_, false) == 0)
                continue;
#elif AB_UNIX
            if (path_.Compare(root_, true) == 0)
                continue;
#endif
        }
        newEntry.name_ = directories[i];
        newEntry.directory_ = true;
        fileEntries_.Push(newEntry);
    }

    for (unsigned i = 0; i < files.Size(); ++i)
    {
        FileSelectorEntry newEntry;
        newEntry.name_ = files[i];
        newEntry.directory_ = false;
        fileEntries_.Push(newEntry);
    }

    Sort(fileEntries_.Begin(), fileEntries_.End(), CompareEntries);

    UIElement* listContent = fileList_->GetContentElement();
    listContent->DisableLayoutUpdate();
    for (unsigned i = 0; i < fileEntries_.Size(); ++i)
    {
        String displayName;
        if (fileEntries_[i].directory_)
            displayName = "<DIR> " + fileEntries_[i].name_;
        else
            displayName = fileEntries_[i].name_;

        Text* entryText = fileList_->CreateChild<Text>();
        fileList_->AddItem(entryText);
        entryText->SetText(displayName);
        entryText->SetStyle("FileSelectorListText");
    }
    listContent->EnableLayoutUpdate();
    listContent->UpdateLayout();
}

bool FilePicker::EnterFile()
{
    unsigned index = fileList_->GetSelection();
    if (index >= fileEntries_.Size())
        return false;

    if (fileEntries_[index].directory_)
    {
        // If a directory double clicked, enter it. Recognize . and .. as a special case
        const String& newPath = fileEntries_[index].name_;
        if ((newPath != ".") && (newPath != ".."))
            SetPath(path_ + newPath);
        else if (newPath == "..")
        {
            String parentPath = GetParentPath(path_);
            SetPath(parentPath);
        }

        return true;
    }
    else
    {
        using namespace FilePicked;

        VariantMap& eventData = GetEventDataMap();
        eventData[P_FILENAME] = path_ + fileEntries_[index].name_;
        SendEvent(E_FILEPICKED, eventData);
        Close();
    }

    return false;
}
