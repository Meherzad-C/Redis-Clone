#pragma once

#include <set>
#include <cstdint>
#include <assert.h>
#include <stddef.h>

#include "AVLNode.h"
#include "..\..\Common\Utility.h"

class AVLTree 
{
private:
    AVLNode* root;

private:
    uint32_t AvlT_Depth(AVLNode* node);
    uint32_t AvlT_Count(AVLNode* node);
    void AvlT_Update(AVLNode* node);
    AVLNode* AvlT_RotateLeft(AVLNode* node);
    AVLNode* AvlT_RotateRight(AVLNode* node);
    AVLNode* AvlT_FixLeft(AVLNode* root);
    AVLNode* AvlT_FixRight(AVLNode* root);
    AVLNode* AvlT_Fix(AVLNode* node);
    AVLNode* AvlT_Delete(AVLNode* node);
    void AvlT_Verify(AVLNode* parent, AVLNode* node);
    void AvlT_Extract(AVLNode* node, std::multiset<uint32_t>& extracted);
    AVLNode* Avlt_Offset(AVLNode* node, int64_t offset);

public:
    AVLTree();
    void Init(AVLNode* node);
    void Add(uint32_t val);
    bool Del(uint32_t val);
    void Verify(const std::multiset<uint32_t>& ref);
    void Dispose();
};