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

    void ParseString(const String& value);
    void GetBatches(PODVector<UIBatch>& batches, PODVector<float>& vertexData, const IntRect& currentScissor) override;
private:
    void ParseNode(const pugi::xml_node& pugiNode, TreeNode* treeNode);
    void DeleteTree(TreeNode* tree);
    String GetTreeText() const;
    String GetNodeText(TreeNode* node) const;
    TreeNode* tree_{ nullptr };
};