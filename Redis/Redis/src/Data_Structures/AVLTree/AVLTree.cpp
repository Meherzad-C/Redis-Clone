#include "AVLTree.h"

struct Data 
{
    AVLNode node;
    uint32_t value;

    Data(uint32_t v) : value(v) {}
};

AVLTree::AVLTree() : root(nullptr) 
{
}

void AVLTree::Init(AVLNode* node) 
{
    node->depth = 1;
    node->count = 1;
    node->left = node->right = node->parent = nullptr;
}

uint32_t AVLTree::AvlT_Depth(AVLNode* node) 
{
    return node ? node->depth : 0;
}

uint32_t AVLTree::AvlT_Count(AVLNode* node) 
{
    return node ? node->count : 0;
}

void AVLTree::AvlT_Update(AVLNode* node) 
{
    node->depth = 1 + _Max(AvlT_Depth(node->left), AvlT_Depth(node->right));
    node->count = 1 + AvlT_Count(node->left) + AvlT_Count(node->right);
}

AVLNode* AVLTree::AvlT_RotateLeft(AVLNode* node) 
{
    AVLNode* new_node = node->right;
    if (new_node->left) 
    {
        new_node->left->parent = node;
    }
    node->right = new_node->left;
    new_node->left = node;
    new_node->parent = node->parent;
    node->parent = new_node;
    AvlT_Update(node);
    AvlT_Update(new_node);
    return new_node;
}

AVLNode* AVLTree::AvlT_RotateRight(AVLNode* node) 
{
    AVLNode* new_node = node->left;
    if (new_node->right) 
    {
        new_node->right->parent = node;
    }
    node->left = new_node->right;
    new_node->right = node;
    new_node->parent = node->parent;
    node->parent = new_node;
    AvlT_Update(node);
    AvlT_Update(new_node);
    return new_node;
}

AVLNode* AVLTree::AvlT_FixLeft(AVLNode* root) 
{
    if (AvlT_Depth(root->left->left) < AvlT_Depth(root->left->right)) 
    {
        root->left = AvlT_RotateLeft(root->left);
    }
    return AvlT_RotateRight(root);
}

AVLNode* AVLTree::AvlT_FixRight(AVLNode* root) 
{
    if (AvlT_Depth(root->right->right) < AvlT_Depth(root->right->left))
    {
        root->right = AvlT_RotateRight(root->right);
    }
    return AvlT_RotateLeft(root);
}

AVLNode* AVLTree::AvlT_Fix(AVLNode* node) 
{
    while (true) 
    {
        AvlT_Update(node);
        uint32_t l = AvlT_Depth(node->left);
        uint32_t r = AvlT_Depth(node->right);
        AVLNode** from = nullptr;
        if (node->parent) 
        {
            from = (node->parent->left == node) ? &node->parent->left : &node->parent->right;
        }
        if (l == r + 2) 
        {
            node = AvlT_FixLeft(node);
        }
        else if (l + 2 == r) 
        {
            node = AvlT_FixRight(node);
        }
        if (!from) 
        {
            return node;
        }
        *from = node;
        node = node->parent;
    }
}

AVLNode* AVLTree::AvlT_Delete(AVLNode* node) 
{
    if (node->right == nullptr) 
    {
        AVLNode* parent = node->parent;
        if (node->left) 
        {
            node->left->parent = parent;
        }
        if (parent) 
        {
            (parent->left == node ? parent->left : parent->right) = node->left;
            return AvlT_Fix(parent);
        }
        else 
        {
            return node->left;
        }
    }
    else 
    {
        AVLNode* victim = node->right;
        while (victim->left) 
        {
            victim = victim->left;
        }
        AVLNode* root = AvlT_Delete(victim);

        *victim = *node;
        if (victim->left) 
        {
            victim->left->parent = victim;
        }
        if (victim->right) 
        {
            victim->right->parent = victim;
        }
        AVLNode* parent = node->parent;
        if (parent) 
        {
            (parent->left == node ? parent->left : parent->right) = victim;
            return root;
        }
        else 
        {
            return victim;
        }
    }
}

void AVLTree::Add(uint32_t val) 
{
    Data* data = new Data(val);
    Init(&data->node);

    AVLNode* cur = nullptr;
    AVLNode** from = &root;
    while (*from) 
    {
        cur = *from;
        uint32_t node_val = container_of(cur, Data, node)->value;
        from = (val < node_val) ? &cur->left : &cur->right;
    }
    *from = &data->node;
    data->node.parent = cur;
    root = AvlT_Fix(&data->node);
}

bool AVLTree::Del(uint32_t val) 
{
    AVLNode* cur = root;
    while (cur) 
    {
        uint32_t node_val = container_of(cur, Data, node)->value;
        if (val == node_val) 
        {
            break;
        }
        cur = val < node_val ? cur->left : cur->right;
    }
    if (!cur) 
    {
        return false;
    }

    root = AvlT_Delete(cur);
    delete container_of(cur, Data, node);
    return true;
}

void AVLTree::AvlT_Verify(AVLNode* parent, AVLNode* node) 
{
    if (!node) 
    {
        return;
    }

    assert(node->parent == parent);
    AvlT_Verify(node, node->left);
    AvlT_Verify(node, node->right);

    assert(node->count == 1 + AvlT_Count(node->left) + AvlT_Count(node->right));

    uint32_t l = AvlT_Depth(node->left);
    uint32_t r = AvlT_Depth(node->right);
    assert(l == r || l + 1 == r || l == r + 1);
    assert(node->depth == 1 + _Max(l, r));

    uint32_t val = container_of(node, Data, node)->value;
    if (node->left) 
    {
        assert(node->left->parent == node);
        assert(container_of(node->left, Data, node)->value <= val);
    }
    if (node->right) 
    {
        assert(node->right->parent == node);
        assert(container_of(node->right, Data, node)->value >= val);
    }
}

void AVLTree::AvlT_Extract(AVLNode* node, std::multiset<uint32_t>& extracted) 
{
    if (!node) 
    {
        return;
    }
    AvlT_Extract(node->left, extracted);
    extracted.insert(container_of(node, Data, node)->value);
    AvlT_Extract(node->right, extracted);
}

AVLNode* AVLTree::Avlt_Offset(AVLNode* node, int64_t offset)
{
    int64_t pos = 0;    // relative to the starting node
    while (offset != pos) 
    {
        if (pos < offset && pos + AvlT_Count(node->right) >= offset) 
        {
            // the target is inside the right subtree
            node = node->right;
            pos += AvlT_Count(node->left) + 1;
        }
        else if (pos > offset && pos - AvlT_Count(node->left) <= offset) 
        {
            // the target is inside the left subtree
            node = node->left;
            pos -= AvlT_Count(node->right) + 1;
        }
        else 
        {
            // go to the parent
            AVLNode* parent = node->parent;
            if (!parent) 
            {
                return NULL;
            }
            if (parent->right == node) 
            {
                pos -= AvlT_Count(node->left) + 1;
            }
            else 
            {
                pos += AvlT_Count(node->right) + 1;
            }
            node = parent;
        }
    }
    return node;
}

void AVLTree::Verify(const std::multiset<uint32_t>& ref) 
{
    AvlT_Verify(nullptr, root);
    assert(AvlT_Count(root) == ref.size());
    std::multiset<uint32_t> extracted;
    AvlT_Extract(root, extracted);
    assert(extracted == ref);
}

void AVLTree::Dispose()
{
    while (root) 
    {
        AVLNode* node = root;
        root = AvlT_Delete(root);
        delete container_of(node, Data, node);
    }
}