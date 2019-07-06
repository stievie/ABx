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
    if (_stricmp(name, "a") == 0)
    {
        // Link
        TreeNode* child = new TreeNode();
        child->type = NodeType::Link;
        child->value = pugiNode.attribute("href").as_string();
        treeNode->children.Push(child);
        nd = child;
    }
    else if (_stricmp(name, "color") == 0)
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
