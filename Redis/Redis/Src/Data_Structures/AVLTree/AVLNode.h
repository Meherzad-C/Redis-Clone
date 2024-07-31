#pragma once

#include <stddef.h>
#include <stdint.h>

// AVLNode for AVLTree
class AVLNode 
{
public:
    uint32_t depth;
    uint32_t count;
    AVLNode* left;
    AVLNode* right;
    AVLNode* parent;

public:
    AVLNode();
};