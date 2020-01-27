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

enum class NodeType
{
    Root,
    Text,
    Color,
    Link
};

struct TreeNode
{
    NodeType type{ NodeType::Root };
    String value;
    PODVector<TreeNode*> children;
};

class FormatText : public Text
{
    URHO3D_OBJECT(FormatText, Text);
public:
    static void RegisterObject(Context* context);

    FormatText(Context* context);
    ~FormatText();

    void SetMarkupText(const String& value);
    void GetBatches(PODVector<UIBatch>& batches, PODVector<float>& vertexData, const IntRect& currentScissor) override;
private:
    void ParseNode(const pugi::xml_node& pugiNode, TreeNode* treeNode);
    void DeleteTree(TreeNode* tree);
    String GetTreeText() const;
    String GetNodeText(TreeNode* node) const;
    TreeNode* tree_{ nullptr };
};