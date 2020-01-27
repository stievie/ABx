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

#include "stdafx.h"
#include "FormatText.h"
#include <Urho3D/ThirdParty/PugiXml/pugixml.hpp>

void FormatText::RegisterObject(Context* context)
{
    context->RegisterFactory<FormatText>();
}

FormatText::FormatText(Context* context) :
    Text(context)
{
}

FormatText::~FormatText()
{
    DeleteTree(tree_);
}

void FormatText::SetMarkupText(const String& value)
{
    DeleteTree(tree_);

    tree_ = new TreeNode();
    pugi::xml_document doc;
    if (!doc.load_string(value.CString(), value.Length()))
    {
        // Simple text
        tree_->type = NodeType::Text;
        tree_->value = value;
    }
    else
    {
        // Seems to be markup
        tree_->type = NodeType::Root;
        for (const auto& node : doc.children())
            ParseNode(node, tree_);
    }

    String text = GetNodeText(tree_);
    SetText(text);
}

void FormatText::GetBatches(PODVector<UIBatch>& batches, PODVector<float>& vertexData, const IntRect& currentScissor)
{
    Text::GetBatches(batches, vertexData, currentScissor);
}

void FormatText::ParseNode(const pugi::xml_node& pugiNode, TreeNode* treeNode)
{
    const char* name = pugiNode.name();
    TreeNode* nd = treeNode;
    if (strcmp(name, "a") == 0)
    {
        // Link
        TreeNode* child = new TreeNode();
        child->type = NodeType::Link;
        child->value = pugiNode.attribute("href").as_string();
        treeNode->children.Push(child);
        nd = child;
    }
    else if (strcmp(name, "color") == 0)
    {
        // Font color
        TreeNode* child = new TreeNode();
        child->type = NodeType::Color;
        child->value = pugiNode.attribute("value").as_string();
        treeNode->children.Push(child);
        nd = child;
    }
    else
    {
        TreeNode* child = new TreeNode();
        child->type = NodeType::Text;
        child->value = pugiNode.value();
        treeNode->children.Push(child);
        nd = child;
    }

    for (const auto& node : pugiNode.children())
        ParseNode(node, nd);
}

void FormatText::DeleteTree(TreeNode* tree)
{
    if (!tree)
        return;

    for (auto* node : tree->children)
        DeleteTree(node);
    delete tree;
}

String FormatText::GetTreeText() const
{
    return GetNodeText(tree_);
}

String FormatText::GetNodeText(TreeNode* node) const
{
    if (!node)
        return String::EMPTY;

    String result;
    if (node->type == NodeType::Text)
        result = node->value;
    for (TreeNode* child : node->children)
    {
        String value = GetNodeText(child);
        if (!value.Empty())
        {
            if (!result.Empty())
                result += " ";
            result += value;
        }
    }
    return result;
}
