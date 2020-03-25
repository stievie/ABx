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

#pragma once

#include "DialogWindow.h"

URHO3D_EVENT(E_FILEPICKED, FilePicked)
{
    URHO3D_PARAM(P_FILENAME, FileName);              // String
}

class FilePicker : public DialogWindow
{
    URHO3D_OBJECT(FilePicker, DialogWindow)
public:
    enum class Mode
    {
        Save,
        Load
    };
    FilePicker(Context* context, const String& root, Mode mode);
    void SetPath(const String& path);
private:
    void HandleCancelClicked(StringHash eventType, VariantMap& eventData);
    void HandleOkClicked(StringHash eventType, VariantMap& eventData);
    void HandleFileSelected(StringHash eventType, VariantMap& eventData);
    void HandleFileDoubleClicked(StringHash eventType, VariantMap& eventData);
    void ScanPath();
    void EnterFile();
    String root_;
    String path_;
    Mode mode_;
    ListView* fileList_{ nullptr };
    LineEdit* fileNameEdit_;
    Vector<FileSelectorEntry> fileEntries_;
};
